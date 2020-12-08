#include "DataConfig/Deserialize/Handlers/DcClassDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "UObject/Package.h"

namespace DcHandlers {

FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::ClassRoot, &bWritePass));

	bool bPropertyPass = Ctx.TopProperty()->IsA<UClass>();
	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		FDcObjectPropertyStat WriteClassStat{ Ctx.TopProperty()->GetFName(), EDcObjectPropertyControl::ExpandObject };
		DC_TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

		EDcDataEntry CurPeek;
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&CurPeek));

			//	read key
			if (CurPeek == EDcDataEntry::MapEnd)
			{
				DC_TRY(Ctx.Reader->ReadMapEnd());
				break;
			}
			else if (CurPeek == EDcDataEntry::String)
			{
				FString Value;
				DC_TRY(Ctx.Reader->ReadString(&Value));
				if (DcDeserializeUtils::IsMeta(Value))
				{
					DC_TRY(DcReadNextExpect(*Ctx.Reader, EDcDataEntry::String));
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
				return DC_FAIL(DcDDeserialize, DataEntryMismatch2)
					<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
			}

			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
		}

		DC_TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::MapRoot << Next;
	}
}

static FDcResult LoadObjectByPath(FObjectProperty* ObjectProperty, UClass* LoadClass, FString LoadPath, FDcDeserializeContext& Ctx, UObject*& OutLoaded)
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
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::String
		|| Next == EDcDataEntry::Nil
		|| Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::ClassRoot, &bWritePass));

	bool bPropertyPass = Ctx.TopProperty()->IsA<FObjectProperty>();
	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	FObjectProperty* ObjectProperty = Cast<FObjectProperty>(Ctx.TopProperty());

	FDcObjectPropertyStat RefStat {
		ObjectProperty->PropertyClass->GetFName(), EDcObjectPropertyControl::ExternalReference
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
			if (FObjectPropertyBase::ParseObjectPropertyValue(
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
					DC_TRY(Ctx.Writer->WriteObjectReference(Loaded));
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
			DC_TRY(Ctx.Writer->WriteObjectReference(Loaded));
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
		DC_TRY(Ctx.Writer->WriteObjectReference(Loaded));
		DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

		return DcOkWithProcessed(OutRet);
	}
	else if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
		DC_TRY(Ctx.Writer->WriteNil());
		DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch3)
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::String << Next;
	}
}

static bool IsSubObjectProperty(FObjectProperty* ObjectProperty)
{
	//	check `UPROPERTY(Instanced)`
	return ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference)
	//	check UCLASS(DefaultToInstanced, EditInlineNew)
		|| ObjectProperty->PropertyClass->HasAnyClassFlags(CLASS_EditInlineNew | CLASS_DefaultToInstanced);
}

EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx)
{
	FObjectProperty* ObjectProperty = Cast<FObjectProperty>(Ctx.TopProperty());
	return ObjectProperty && IsSubObjectProperty(ObjectProperty)
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	FDcPutbackReader PutbackReader(Ctx.Reader);

	EDcDataEntry Next;
	DC_TRY(PutbackReader.PeekRead(&Next));

	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::ClassRoot, &bWritePass));

	FObjectProperty* ObjectProperty = Cast<FObjectProperty>(Ctx.TopProperty());
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

	//	TODO this is now becoming a bit verbose, needs something around this
	DC_TRY(PutbackReader.PeekRead(&Next));
	if (Next != EDcDataEntry::String)
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}

	FString MetaKey;
	DC_TRY(PutbackReader.ReadString(&MetaKey));
	if (MetaKey == TEXT("$type"))
	{
		DC_TRY(PutbackReader.PeekRead(&Next));
		if (Next != EDcDataEntry::String)
		{
			return DC_FAIL(DcDDeserialize, DataEntryMismatch)
				<< EDcDataEntry::String << Next;
		}

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
	DC_TRY(Ctx.Writer->WriteDataEntry(FObjectProperty::StaticClass(), Datum));

	FObjectProperty* SubObjectProperty = Datum.Cast<FObjectProperty>();
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

	FDcObjectPropertyStat WriteClassStat{ ObjectProperty->GetFName(), EDcObjectPropertyControl::ExpandObject };
	DC_TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

	//	usual read routine
	EDcDataEntry CurPeek;
	DC_TRY(PutbackReader.PeekRead(&CurPeek));
	while (CurPeek != EDcDataEntry::MapEnd)
	{
		if (CurPeek == EDcDataEntry::Name)
		{
			FName Value;
			DC_TRY(PutbackReader.ReadName(&Value));
			DC_TRY(Ctx.Writer->WriteName(Value));
		}
		else if (CurPeek == EDcDataEntry::String)
		{
			FString Value;
			DC_TRY(PutbackReader.ReadString(&Value));
			DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		}
		else
		{
			return DC_FAIL(DcDDeserialize, DataEntryMismatch2)
				<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
		}

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

		DC_TRY(PutbackReader.PeekRead(&CurPeek));
	}

	DC_TRY(PutbackReader.ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

	check(PutbackReader.Cached.Num() == 0);
	return DcOkWithProcessed(OutRet);
}

}	// namespace DcHandlers

