#include "DataConfig/Deserialize/Handlers/DcClassDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "UObject/Package.h"

namespace DcHandlers {

FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDcDataEntry::ClassRoot).Ok();
	bool bPropertyPass = Ctx.TopProperty()->IsA<UClass>();

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		FDcClassPropertyStat WriteClassStat{ Ctx.TopProperty()->GetFName(), EDcDataReference::ExpandObject };
		DC_TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

		EDcDataEntry CurPeek = Ctx.Reader->Peek();
		while (CurPeek != EDcDataEntry::MapEnd)
		{
			//	read key
			if (CurPeek == EDcDataEntry::Name)
			{
				DC_TRY(Ctx.Writer->Peek(EDcDataEntry::Name));

				FName Value;
				DC_TRY(Ctx.Reader->ReadName(&Value));
				if (DcIsMeta(Value))
				{
					DC_TRY(Ctx.Reader->ReadName(nullptr));
					continue;
				}
				else
				{
					DC_TRY(Ctx.Writer->WriteName(Value));
				}
			}
			else if (CurPeek == EDcDataEntry::String)
			{
				DC_TRY(Ctx.Writer->Peek(EDcDataEntry::Name));

				FString Value;
				DC_TRY(Ctx.Reader->ReadString(&Value));
				if (DcIsMeta(Value))
				{
					DC_TRY(Ctx.Reader->ReadString(nullptr));
					continue;
				}
				else
				{
					DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
				}
			}
			else
			{
				return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch2))
					<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
			}

			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

			CurPeek = Ctx.Reader->Peek();
		}

		DC_TRY(Ctx.Reader->ReadMapEnd());
		DC_TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch))
			<< EDcDataEntry::MapRoot << Next;
	}
}

static FDcResult LoadObjectByPath(UObjectProperty* ObjectProperty, UClass* LoadClass, FString LoadPath, FDcDeserializeContext& Ctx, UObject*& OutLoaded)
{
	DC_TRY(DcExpect(LoadClass != nullptr));
	DC_TRY(DcExpect(LoadClass->IsChildOf(ObjectProperty->PropertyClass)));
	UObject* Loaded = StaticLoadObject(LoadClass, nullptr, *LoadPath, nullptr);
	DC_TRY(DcExpect(Loaded != nullptr));

	OutLoaded = Loaded;
	return DcOk();
}


FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDcDataEntry::String
		|| Next == EDcDataEntry::Nil
		|| Next == EDcDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDcDataEntry::ClassRoot).Ok();
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Ctx.TopProperty());
	bool bPropertyPass = Ctx.TopProperty()->IsA<UObjectProperty>();

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	FDcClassPropertyStat RefStat {
		ObjectProperty->PropertyClass->GetFName(), EDcDataReference::ExternalReference
	};
	FDcClassPropertyStat NullStat {
		ObjectProperty->PropertyClass->GetFName(), EDcDataReference::NullReference
	};

	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

		if (Value.EndsWith(TEXT("'")))
		{
			//	SkeletalMesh'/Engine/EditorMeshes/SkeletalMesh/DefaultSkeletalMesh.DefaultSkeletalMesh'
			//	UE4 copied reference style
			//	ref: FPropertyHandleObject::SetValueFromFormattedString
			UObject* Loaded = nullptr;
			const TCHAR* ValueBuffer = *Value;
			if (UObjectPropertyBase::ParseObjectPropertyValue(
				ObjectProperty,
				nullptr,	// see PropertyHandleImpl.cpp it's using nullptr
				ObjectProperty->PropertyClass,
				0,
				ValueBuffer,
				Loaded))
			{
				if (Loaded)
				{
					DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
					DC_TRY(Ctx.Writer->WriteReference(Loaded));
					DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

					return DcOkWithProcessed(OutRet);
				}
			}

			return DcFail();
		}
		else if (Value.StartsWith(TEXT("/")))
		{
			//	/Game/Path/To/Object
			//	`Game` is a Mount Point
			UObject* Loaded = nullptr;
			DC_TRY(LoadObjectByPath(ObjectProperty, ObjectProperty->PropertyClass, Value, Ctx, Loaded));

			DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
			DC_TRY(Ctx.Writer->WriteReference(Loaded));
			DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

			return DcOkWithProcessed(OutRet);
		}
		else
		{
			return DcFail();
		}
	}
	else if (Next == EDcDataEntry::MapRoot)
	{
		//	{
		//		"$type" : "FooType",
		//		"$path" : "/Game/Path/To/Object",
		//	}
		//
		//	note that this is ordred and type and path needs to be first 2 items

		DC_TRY(Ctx.Reader->ReadMapRoot());
		FString MetaKey;
		DC_TRY(Ctx.Reader->ReadString(&MetaKey));
		DC_TRY(DcExpect(MetaKey == TEXT("$type")));

		FString LoadClassName;
		DC_TRY(Ctx.Reader->ReadString(&LoadClassName));

		DC_TRY(Ctx.Reader->ReadString(&MetaKey));
		DC_TRY(DcExpect(MetaKey == TEXT("$path")));

		FString LoadPath;
		DC_TRY(Ctx.Reader->ReadString(&LoadPath));
		DC_TRY(Ctx.Reader->ReadMapEnd());

		UClass* LoadClass = FindObject<UClass>(ANY_PACKAGE, *LoadClassName, true);
		DC_TRY(DcExpect(LoadClass != nullptr));

		UObject* Loaded = nullptr;
		DC_TRY(LoadObjectByPath(ObjectProperty, LoadClass, LoadPath, Ctx, Loaded));

		DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
		DC_TRY(Ctx.Writer->WriteReference(Loaded));
		DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

		return DcOkWithProcessed(OutRet);
	}
	else if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Writer->WriteClassRoot(NullStat));
		DC_TRY(Ctx.Writer->WriteNil());
		DC_TRY(Ctx.Writer->WriteClassEnd(NullStat));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch3))
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::String << Next;
	}
}

static bool IsSubObjectProperty(UObjectProperty* ObjectProperty)
{
	//	check `UPROPERTY(Instanced)`
	return ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference)
	//	check UCLASS(DefaultToInstanced, EditInlineNew)
		|| ObjectProperty->PropertyClass->HasAnyClassFlags(CLASS_EditInlineNew | CLASS_DefaultToInstanced);
}

EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx)
{
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Ctx.TopProperty());
	return ObjectProperty && IsSubObjectProperty(ObjectProperty)
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	FDcPutbackReader PutbackReader(Ctx.Reader);

	EDcDataEntry Next = PutbackReader.Peek();
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;
	bool bWritePass = Ctx.Writer->Peek(EDcDataEntry::ClassRoot).Ok();

	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Ctx.TopProperty());
	bool bPropertyPass = ObjectProperty != nullptr;

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	if (!IsSubObjectProperty(ObjectProperty))
	{
		return DcFail();
	}

	DC_TRY(PutbackReader.ReadMapRoot());

	UClass* SubClassType = nullptr;

	FString MetaKey;
	DC_TRY(PutbackReader.ReadString(&MetaKey));
	if (MetaKey == TEXT("$type"))
	{
		//	has `$type`
		FString TypeStr;
		DC_TRY(PutbackReader.ReadString(&TypeStr));

		//	TODO Core won't link Engine, move this into the Editor class or a new one like `DataConfigEngine`
		//		console Program linking Engine just doesn't work
		/*
		if (TypeStr.StartsWith(TEXT("/")))
		{
			//	"/Game/Path/To/Blueprint"
			//	It's a path, try load blueprint and use `GeneratedClass`
			UObject* Loaded = StaticLoadObject(UBlueprint::StaticClass(), nullptr, *TypeStr, nullptr);
			if (!Loaded)
				return Fail();
			UBlueprint* BP = Cast<UBlueprint>(Loaded);
			if (!BP)
				return Fail();

			SubClassType = BP->GeneratedClass;
		}
		else
		*/

		//	"Character"
		//	Plain class name, note that 'U' is automatically stripped
		SubClassType = FindObject<UClass>(ANY_PACKAGE, *TypeStr, true);
	}
	else
	{
		//	it has not `$type`, use object property's class
		SubClassType = ObjectProperty->PropertyClass;
		PutbackReader.Putback(MetaKey);
	}

	if (!SubClassType)
		return DcFail();

	//	can't be abstract
	if (SubClassType->HasAnyClassFlags(CLASS_Abstract))
		return DcFail();

	if (!SubClassType->IsChildOf(ObjectProperty->PropertyClass))
		return DcFail();

	//	construct the item
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(UObjectProperty::StaticClass(), Datum));

	UObjectProperty* SubObjectProperty = Datum.Cast<UObjectProperty>();
	if (!SubObjectProperty)
		return DcFail();

	UObject* SubObject = SubObjectProperty->GetPropertyValue(Datum.DataPtr);
	if (SubObject != nullptr
		&& SubObject->GetClass() != SubClassType)
	{
		//	existing subobject class mistmatch, destroy existing one and create new one
		SubObject->ConditionalBeginDestroy();
		SubObject = nullptr;
	}

	if (SubObject == nullptr)
		SubObject = NewObject<UObject>(Ctx.TopObject(), SubClassType);

	if (SubObject == nullptr)
		return DcFail();

	ObjectProperty->SetPropertyValue(Datum.DataPtr, SubObject);
	//	manually setting class state
	Ctx.Writer->PushTopClassPropertyState(Datum);

	FDcClassPropertyStat WriteClassStat{ ObjectProperty->GetFName(), EDcDataReference::ExpandObject };
	DC_TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

	//	usual read coroutine
	EDcDataEntry CurPeek = PutbackReader.Peek();
	while (CurPeek != EDcDataEntry::MapEnd)
	{
		if (CurPeek == EDcDataEntry::Name)
		{
			DC_TRY(Ctx.Writer->Peek(EDcDataEntry::Name));

			FName Value;
			DC_TRY(PutbackReader.ReadName(&Value));
			DC_TRY(Ctx.Writer->WriteName(Value));
		}
		else if (CurPeek == EDcDataEntry::String)
		{
			DC_TRY(Ctx.Writer->Peek(EDcDataEntry::Name));

			FString Value;
			DC_TRY(PutbackReader.ReadString(&Value));
			DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		}
		else
		{
			return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch2))
				<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
		}

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

		CurPeek = PutbackReader.Peek();
	}

	DC_TRY(PutbackReader.ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

	check(PutbackReader.Cached.Num() == 0);
	return DcOkWithProcessed(OutRet);
}

}	// namespace DcHandlers

