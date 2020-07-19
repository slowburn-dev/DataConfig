#include "DataConfig/Deserialize/Handlers/DcClassDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "UObject/Package.h"

namespace DataConfig
{

FResult HandlerClassRootDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	EDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDataEntry::ClassRoot).Ok();
	bool bPropertyPass = Ctx.TopProperty()->IsA<UClass>();

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return OkWithCanNotProcess(OutRet);
	}

	if (Next == EDataEntry::MapRoot)
	{
		TRY(Ctx.Reader->ReadMapRoot(nullptr));
		FClassPropertyStat WriteClassStat{ Ctx.TopProperty()->GetFName(), EDataReference::ExpandObject };
		TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

		EDataEntry CurPeek = Ctx.Reader->Peek();
		while (CurPeek != EDataEntry::MapEnd)
		{
			//	read key
			if (CurPeek == EDataEntry::Name)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FName Value;
				TRY(Ctx.Reader->ReadName(&Value, nullptr));
				if (IsMeta(Value))
				{
					TRY(Ctx.Reader->ReadName(nullptr, nullptr));
					continue;
				}
				else
				{
					TRY(Ctx.Writer->WriteName(Value));
				}
			}
			else if (CurPeek == EDataEntry::String)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FString Value;
				TRY(Ctx.Reader->ReadString(&Value, nullptr));
				if (IsMeta(Value))
				{
					TRY(Ctx.Reader->ReadString(nullptr, nullptr));
					continue;
				}
				else
				{
					TRY(Ctx.Writer->WriteName(FName(*Value)));
				}
			}
			else
			{
				return Fail(EErrorCode::DeserializeTypeNotMatch);
			}

			FScopedProperty ScopedValueProperty(Ctx);
			TRY(ScopedValueProperty.PushProperty());
			TRY(Ctx.Deserializer->Deserialize(Ctx));

			CurPeek = Ctx.Reader->Peek();
		}

		TRY(Ctx.Reader->ReadMapEnd(nullptr));
		TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

		return OkWithProcessed(OutRet);
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

static FResult LoadObjectByPath(UObjectProperty* ObjectProperty, UClass* LoadClass, FString LoadPath, FDeserializeContext& Ctx, UObject*& OutLoaded)
{
	TRY(Expect(LoadClass != nullptr, EErrorCode::UnknownError));
	TRY(Expect(LoadClass->IsChildOf(ObjectProperty->PropertyClass), EErrorCode::UnknownError));
	UObject* Loaded = StaticLoadObject(LoadClass, nullptr, *LoadPath, nullptr);
	TRY(Expect(Loaded != nullptr, EErrorCode::UnknownError));

	OutLoaded = Loaded;
	return Ok();
}


FResult HandlerObjectReferenceDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	EDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDataEntry::String
		|| Next == EDataEntry::Nil
		|| Next == EDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDataEntry::ClassRoot).Ok();
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Ctx.TopProperty());
	bool bPropertyPass = Ctx.TopProperty()->IsA<UObjectProperty>();

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return OkWithCanNotProcess(OutRet);
	}

	FClassPropertyStat RefStat {
		ObjectProperty->PropertyClass->GetFName(), EDataReference::ExternalReference
	};
	FClassPropertyStat NullStat {
		ObjectProperty->PropertyClass->GetFName(), EDataReference::NullReference
	};

	if (Next == EDataEntry::String)
	{
		FString Value;
		TRY(Ctx.Reader->ReadString(&Value, nullptr));

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
					TRY(Ctx.Writer->WriteClassRoot(RefStat));
					TRY(Ctx.Writer->WriteReference(Loaded));
					TRY(Ctx.Writer->WriteClassEnd(RefStat));

					return OkWithProcessed(OutRet);
				}
			}

			return Fail(EErrorCode::UnknownError);
		}
		else if (Value.StartsWith(TEXT("/")))
		{
			//	/Game/Path/To/Object
			//	`Game` is a Mount Point
			UObject* Loaded = nullptr;
			TRY(LoadObjectByPath(ObjectProperty, ObjectProperty->PropertyClass, Value, Ctx, Loaded));

			TRY(Ctx.Writer->WriteClassRoot(RefStat));
			TRY(Ctx.Writer->WriteReference(Loaded));
			TRY(Ctx.Writer->WriteClassEnd(RefStat));

			return OkWithProcessed(OutRet);
		}
		else
		{
			return Fail(EErrorCode::UnknownError);
		}
	}
	else if (Next == EDataEntry::MapRoot)
	{
		//	{
		//		"$type" : "FooType",
		//		"$path" : "/Game/Path/To/Object",
		//	}
		//
		//	note that this is ordred and type and path needs to be first 2 items

		TRY(Ctx.Reader->ReadMapRoot(nullptr));
		FString MetaKey;
		TRY(Ctx.Reader->ReadString(&MetaKey, nullptr));
		TRY(Expect(MetaKey == TEXT("$type"), EErrorCode::UnknownError));

		FString LoadClassName;
		TRY(Ctx.Reader->ReadString(&LoadClassName, nullptr));

		TRY(Ctx.Reader->ReadString(&MetaKey, nullptr));
		TRY(Expect(MetaKey == TEXT("$path"), EErrorCode::UnknownError));

		FString LoadPath;
		TRY(Ctx.Reader->ReadString(&LoadPath, nullptr));
		TRY(Ctx.Reader->ReadMapEnd(nullptr));

		UClass* LoadClass = FindObject<UClass>(ANY_PACKAGE, *LoadClassName, true);
		TRY(Expect(LoadClass != nullptr, EErrorCode::UnknownError));

		UObject* Loaded = nullptr;
		TRY(LoadObjectByPath(ObjectProperty, LoadClass, LoadPath, Ctx, Loaded));

		TRY(Ctx.Writer->WriteClassRoot(RefStat));
		TRY(Ctx.Writer->WriteReference(Loaded));
		TRY(Ctx.Writer->WriteClassEnd(RefStat));

		return OkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::Nil)
	{
		TRY(Ctx.Writer->WriteClassRoot(NullStat));
		TRY(Ctx.Writer->WriteNil());
		TRY(Ctx.Writer->WriteClassEnd(NullStat));

		return OkWithProcessed(OutRet);
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

static bool IsSubObjectProperty(UObjectProperty* ObjectProperty)
{
	//	check `UPROPERTY(Instanced)`
	return ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference)
	//	check UCLASS(DefaultToInstanced, EditInlineNew)
		|| ObjectProperty->PropertyClass->HasAnyClassFlags(CLASS_EditInlineNew | CLASS_DefaultToInstanced);
}

EDeserializePredicateResult PredicateIsSubObjectProperty(FDeserializeContext& Ctx)
{
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Ctx.TopProperty());
	return ObjectProperty && IsSubObjectProperty(ObjectProperty)
		? EDeserializePredicateResult::Process
		: EDeserializePredicateResult::Pass;
}

FResult HandlerInstancedSubObjectDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	FPutbackReader PutbackReader(Ctx.Reader);

	EDataEntry Next = PutbackReader.Peek();
	bool bRootPeekPass = Next == EDataEntry::MapRoot;
	bool bWritePass = Ctx.Writer->Peek(EDataEntry::ClassRoot).Ok();

	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Ctx.TopProperty());
	bool bPropertyPass = ObjectProperty != nullptr;

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return OkWithCanNotProcess(OutRet);
	}

	if (!IsSubObjectProperty(ObjectProperty))
	{
		return Fail(EErrorCode::UnknownError);
	}

	TRY(PutbackReader.ReadMapRoot(nullptr));

	UClass* SubClassType = nullptr;

	FString MetaKey;
	TRY(PutbackReader.ReadString(&MetaKey, nullptr));
	if (MetaKey == TEXT("$type"))
	{
		//	has `$type`
		FString TypeStr;
		TRY(PutbackReader.ReadString(&TypeStr, nullptr));

		//	TODO Core won't link Engine, move this into the Editor class or a new one like `DataConfigEngine`
		//		console Program linking Engine just doesn't work
		/*
		if (TypeStr.StartsWith(TEXT("/")))
		{
			//	"/Game/Path/To/Blueprint"
			//	It's a path, try load blueprint and use `GeneratedClass`
			UObject* Loaded = StaticLoadObject(UBlueprint::StaticClass(), nullptr, *TypeStr, nullptr);
			if (!Loaded)
				return Fail(EErrorCode::UnknownError);
			UBlueprint* BP = Cast<UBlueprint>(Loaded);
			if (!BP)
				return Fail(EErrorCode::UnknownError);

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
		return Fail(EErrorCode::UnknownError);

	//	can't be abstract
	if (SubClassType->HasAnyClassFlags(CLASS_Abstract))
		return Fail(EErrorCode::UnknownError);

	if (!SubClassType->IsChildOf(ObjectProperty->PropertyClass))
		return Fail();

	//	construct the item
	FPropertyDatum Datum;
	TRY(Ctx.Writer->WriteDataEntry(UObjectProperty::StaticClass(), EErrorCode::UnknownError, Datum));

	UObjectProperty* SubObjectProperty = Datum.Cast<UObjectProperty>();
	if (!SubObjectProperty)
		return Fail();

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
		return Fail();

	ObjectProperty->SetPropertyValue(Datum.DataPtr, SubObject);
	//	manually setting class state
	Ctx.Writer->PushTopClassPropertyState(Datum);

	FClassPropertyStat WriteClassStat{ ObjectProperty->GetFName(), EDataReference::ExpandObject };
	TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

	//	usual read coroutine
	EDataEntry CurPeek = PutbackReader.Peek();
	while (CurPeek != EDataEntry::MapEnd)
	{
		if (CurPeek == EDataEntry::Name)
		{
			TRY(Ctx.Writer->Peek(EDataEntry::Name));

			FName Value;
			TRY(PutbackReader.ReadName(&Value, nullptr));
			TRY(Ctx.Writer->WriteName(Value));
		}
		else if (CurPeek == EDataEntry::String)
		{
			TRY(Ctx.Writer->Peek(EDataEntry::Name));

			FString Value;
			TRY(PutbackReader.ReadString(&Value, nullptr));
			TRY(Ctx.Writer->WriteName(FName(*Value)));
		}
		else
		{
			return Fail(EErrorCode::DeserializeTypeNotMatch);
		}

		FScopedProperty ScopedValueProperty(Ctx);
		TRY(ScopedValueProperty.PushProperty());
		TRY(Ctx.Deserializer->Deserialize(Ctx));

		CurPeek = PutbackReader.Peek();
	}

	TRY(PutbackReader.ReadMapEnd(nullptr));
	TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

	check(PutbackReader.Cached.Num() == 0);
	return OkWithProcessed(OutRet);
}

} // namespace DataConfig
