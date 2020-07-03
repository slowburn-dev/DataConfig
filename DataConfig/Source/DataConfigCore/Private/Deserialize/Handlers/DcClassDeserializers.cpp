#include "Deserialize/Handlers/DcClassDeserializers.h"
#include "Deserialize/DcDeserializer.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"
#include "UObject/Package.h"

namespace DataConfig
{

FResult DATACONFIGCORE_API DataConfig::ClassRootDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
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
		FClassPropertyStat WriteClassStat{ Ctx.TopProperty()->GetFName(), EDataReference::InlineObject };
		TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

		EDataEntry CurPeek = Ctx.Reader->Peek();
		while (CurPeek != EDataEntry::MapEnd)
		{
			if (CurPeek == EDataEntry::Name)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FName Value;
				TRY(Ctx.Reader->ReadName(&Value, nullptr));
				TRY(Ctx.Writer->WriteName(Value));
			}
			else if (CurPeek == EDataEntry::String)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FString Value;
				TRY(Ctx.Reader->ReadString(&Value, nullptr));
				TRY(Ctx.Writer->WriteName(FName(*Value)));
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


FResult DATACONFIGCORE_API ObjectReferenceDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
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
			//	UE4 copied reference style
			//	SkeletalMesh'/Engine/EditorMeshes/SkeletalMesh/DefaultSkeletalMesh.DefaultSkeletalMesh'
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
			//	"Game/Path/To/Object"
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
		//		['$type'] = 'FooType',
		//		['$path'] = '/Game/Path/To/Object'
		//	}
		//	note that this requires JSON member order

		TRY(Ctx.Reader->ReadMapRoot(nullptr));
		FString KeyValue;
		TRY(Ctx.Reader->ReadString(&KeyValue, nullptr));
		TRY(Expect(KeyValue == TEXT("$type"), EErrorCode::UnknownError));

		FString LoadClassName;
		TRY(Ctx.Reader->ReadString(&LoadClassName, nullptr));

		TRY(Ctx.Reader->ReadString(&KeyValue, nullptr));
		TRY(Expect(KeyValue == TEXT("$path"), EErrorCode::UnknownError));

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

FResult DATACONFIGCORE_API InstancedSubObjectDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	EDataEntry Next = Ctx.Reader->Peek();
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

	TRY(Ctx.Reader->ReadMapRoot(nullptr));
	FName TypeStr;
	TRY(Ctx.Reader->ReadName(&TypeStr, nullptr));
	TRY(Expect(TypeStr == FName(TEXT("$type")), EErrorCode::UnknownError));

	//	what we need to do here is to .. construct the object Out of the writer
	//FPropertyDatum ObjectRefDatum;
	//TRY(Ctx.Reader->ReadString(&LoadClassName, nullptr));




	FClassPropertyStat WriteClassStat{ Ctx.TopProperty()->GetFName(), EDataReference::InlineObject };
	TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));







	return Fail(EErrorCode::UnknownError);
}

} // namespace DataConfig
