#include "DataConfig/Deserialize/Handlers/Json/DcJsonClassDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "UObject/Package.h"

namespace DcJsonHandlers {

FDcResult TryReadObjectReference(FObjectPropertyBase* ObjectProperty, FDcReader* Reader, UObject*& OutObject)
{
	check(ObjectProperty);

	EDcDataEntry Next;
	DC_TRY(Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Reader->ReadString(&Value));

		if (!Value.StartsWith(TEXT("'"))
			&& Value.EndsWith(TEXT("'")))
		{
			//	SkeletalMesh'/Engine/EditorMeshes/SkeletalMesh/DefaultSkeletalMesh.DefaultSkeletalMesh'
			//	UE4 copied reference style
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
					OutObject = Loaded;
					return DcOk();
				}
			}

			return DC_FAIL(DcDDeserialize, UObjectByStrNotFound)
				<< ObjectProperty->PropertyClass->GetFName()
				<< Value;
		}
		else if (Value.StartsWith(TEXT("/")))
		{
			//	/Game/Path/To/Object
			//	`Game` is a Mount Point, and there's no `.uasset` suffix
			UObject* Loaded;
			DC_TRY(DcDeserializeUtils::TryStaticLoadObject(ObjectProperty->PropertyClass, nullptr, *Value, Loaded));

			OutObject = Loaded;
			return DcOk();
		}
		else if (Value.Len() > 1
			&& Value.StartsWith(TEXT("'"))
			&& Value.EndsWith(TEXT("'")))
		{
			//	'/Foo/Bar'
			//	single quoted name, just strip it 
			FString Unquoted = Value.Mid(1, Value.Len() - 2);
			
			UObject* Loaded;
			DC_TRY(DcDeserializeUtils::TryStaticFindObject(UObject::StaticClass(), ANY_PACKAGE, *Value, false, Loaded));

			OutObject = Loaded;
			return DcOk();
		}
		else
		{
			//	Foo
			//	try find by name
			UObject* Loaded;
			DC_TRY(DcDeserializeUtils::TryStaticFindObject(UObject::StaticClass(), ANY_PACKAGE, *Value, false, Loaded));

			return DcOk();
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

		DC_TRY(Reader->ReadMapRoot());
		FString MetaKey;
		DC_TRY(Reader->ReadString(&MetaKey));
		DC_TRY(DcDeserializeUtils::ExpectMetaKey(MetaKey, TEXT("$type")));

		FString LoadClassName;
		DC_TRY(Reader->ReadString(&LoadClassName));

		DC_TRY(Reader->ReadString(&MetaKey));
		DC_TRY(DcDeserializeUtils::ExpectMetaKey(MetaKey, TEXT("$path")));

		FString LoadPath;
		DC_TRY(Reader->ReadString(&LoadPath));
		DC_TRY(Reader->ReadMapEnd());

		UClass* LoadClass;
		DC_TRY(DcDeserializeUtils::TryFindObject<UClass>(ANY_PACKAGE, *LoadClassName, true, LoadClass));

		DC_TRY(DcDeserializeUtils::ExpectLhsChildOfRhs(LoadClass, ObjectProperty->PropertyClass));
		
		UObject* Loaded;
		DC_TRY(DcDeserializeUtils::TryStaticLoadObject(LoadClass, nullptr, *LoadPath, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
	else if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Reader->ReadNil());

		OutObject = nullptr;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch3)
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::String << Next;
	}
}

FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FClassProperty* ClassProperty = DcPropertyUtils::CastFieldVariant<FClassProperty>(Ctx.TopProperty());
	if (ClassProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ClassProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		DC_TRY(Ctx.Writer->WriteClassReference(nullptr));

		return DcOk();
	}
	else if (Next == EDcDataEntry::String)
	{
		FString ClassStr;
		DC_TRY(Ctx.Reader->ReadString(&ClassStr));

		UClass* LoadClass = FindObject<UClass>(ANY_PACKAGE, *ClassStr, true);
		if (LoadClass == nullptr)
			return DC_FAIL(DcDDeserialize, UObjectByStrNotFound) << TEXT("Class") << ClassStr;

		if (!LoadClass->IsChildOf(ClassProperty->MetaClass))
		{
			return DC_FAIL(DcDDeserialize, ClassLhsIsNotChildOfRhs)
				<< LoadClass->GetFName() << ClassProperty->MetaClass->GetFName();
		}

		DC_TRY(Ctx.Writer->WriteClassReference(LoadClass));

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch2)
			<< EDcDataEntry::Nil << EDcDataEntry::String << Next;
	}
}

FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		FDcClassStat WriteClassStat{ Ctx.TopProperty().GetFName(), FDcClassStat::EControl::ExpandObject };
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

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::MapRoot << Next;
	}
}

FDcDeserializeDelegate FObjectReferenceHandlerGenerator::MakeObjectReferenceHandler()
{
	return FDcDeserializeDelegate::CreateLambda([CapObjectReader{FuncObjectReader}](FDcDeserializeContext& Ctx) -> FDcResult
	{
		FObjectProperty* ObjectProperty = DcPropertyUtils::CastFieldVariant<FObjectProperty>(Ctx.TopProperty());
		if (ObjectProperty == nullptr)
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< TEXT("ObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
		}

		FDcClassStat RefStat {
			ObjectProperty->PropertyClass->GetFName(), FDcClassStat::EControl::ReferenceOrNil
		};

		UObject* Loaded;
		DC_TRY(CapObjectReader(ObjectProperty, Ctx.Reader, Loaded));
		
		DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
		if (Loaded)
			DC_TRY(Ctx.Writer->WriteObjectReference(Loaded));
		else
			DC_TRY(Ctx.Writer->WriteNil());
		DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

		return DcOk();
	});
}

FDcDeserializeDelegate FObjectReferenceHandlerGenerator::MakeSoftObjectReferenceHandler()
{
	return FDcDeserializeDelegate::CreateLambda([CapObjectReader{FuncObjectReader}](FDcDeserializeContext& Ctx) -> FDcResult
	{
		FSoftObjectProperty* SoftObjectProperty = DcPropertyUtils::CastFieldVariant<FSoftObjectProperty>(Ctx.TopProperty());
		if (SoftObjectProperty == nullptr)
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< TEXT("SoftObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
		}

		UObject* Loaded;
		DC_TRY(CapObjectReader(SoftObjectProperty, Ctx.Reader, Loaded));

		DC_TRY(Ctx.Writer->WriteSoftObjectReference(Loaded));
		return DcOk();
	});
}

FDcDeserializeDelegate FObjectReferenceHandlerGenerator::MakeWeakObjectReferenceHandler()
{
	return FDcDeserializeDelegate::CreateLambda([CapObjectReader{FuncObjectReader}](FDcDeserializeContext& Ctx) -> FDcResult
	{
		FWeakObjectProperty* WeakObjectProperty = DcPropertyUtils::CastFieldVariant<FWeakObjectProperty>(Ctx.TopProperty());
		if (WeakObjectProperty == nullptr)
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< TEXT("WeakObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
		}

		UObject* Loaded;
		DC_TRY(CapObjectReader(WeakObjectProperty, Ctx.Reader, Loaded));

		DC_TRY(Ctx.Writer->WriteWeakObjectReference(Loaded));
		return DcOk();
	});
}

FDcDeserializeDelegate FObjectReferenceHandlerGenerator::MakeLazyObjectReferenceHandler()
{
	return FDcDeserializeDelegate::CreateLambda([CapObjectReader{FuncObjectReader}](FDcDeserializeContext& Ctx) -> FDcResult
	{
		FLazyObjectProperty* LazyObjectProperty = DcPropertyUtils::CastFieldVariant<FLazyObjectProperty>(Ctx.TopProperty());
		if (LazyObjectProperty == nullptr)
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< TEXT("LazyObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
		}

		UObject* Loaded;
		DC_TRY(CapObjectReader(LazyObjectProperty, Ctx.Reader, Loaded));

		FLazyObjectPtr LazyPtr{Loaded};
		DC_TRY(Ctx.Writer->WriteLazyObjectReference(LazyPtr));
		return DcOk();
	});
}

EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx)
{
	if (Ctx.TopProperty().IsUObject())
		return EDcDeserializePredicateResult::Pass;

	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Ctx.TopProperty().ToFieldUnsafe());
	return ObjectProperty && DcPropertyUtils::IsSubObjectProperty(ObjectProperty)
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx)
{
	FDcPutbackReader PutbackReader(Ctx.Reader);

	EDcDataEntry Next;
	DC_TRY(PutbackReader.PeekRead(&Next));

	FObjectProperty* ObjectProperty = DcPropertyUtils::CastFieldVariant<FObjectProperty>(Ctx.TopProperty());
	if (ObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	if (!DcPropertyUtils::IsSubObjectProperty(ObjectProperty))
	{
		return DC_FAIL(DcDDeserialize, ObjectPropertyNotInline)
			<< ObjectProperty->GetFName() << ObjectProperty->GetClass()->GetFName();
	}

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		FDcClassStat NilStat;
		NilStat.Control = FDcClassStat::EControl::ReferenceOrNil;
		DC_TRY(Ctx.Writer->WriteClassRoot(NilStat));
		DC_TRY(Ctx.Writer->WriteNil());
		DC_TRY(Ctx.Writer->WriteClassEnd(NilStat));

		return DcOk();
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

		//	"Character"
		//	Plain class name, note that 'U' is automatically stripped
		SubClassType = FindObject<UClass>(ANY_PACKAGE, *TypeStr, true);
		if (!SubClassType)
		{
			return DC_FAIL(DcDDeserialize, UObjectByStrNotFound)
				<< TEXT("Class") << TypeStr;
		}
	}
	else
	{
		//	it has not `$type`, use object property's class
		SubClassType = ObjectProperty->PropertyClass;
		PutbackReader.Putback(MetaKey);
	}

	check(SubClassType);

	DC_TRY(DcDeserializeUtils::ExpectNonAbstract(SubClassType));
	DC_TRY(DcDeserializeUtils::ExpectLhsChildOfRhs(SubClassType, ObjectProperty->PropertyClass));

	//	construct the item
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FObjectProperty::StaticClass(), Datum));

	FObjectProperty* SubObjectProperty = Datum.CastField<FObjectProperty>();
	if (!SubObjectProperty)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Datum.Property.GetFName() << Datum.Property.GetClassName();

	//	don't read *Datum.DataPtr as it might be uninitialized, handle reload elsewhere
	UObject* SubObject = NewObject<UObject>(Ctx.TopObject(), SubClassType);

	ObjectProperty->SetPropertyValue(Datum.DataPtr, SubObject);
	//	manually setting class state
	Ctx.Writer->PushTopClassPropertyState(Datum);

	FDcClassStat WriteClassStat{ ObjectProperty->GetFName(), FDcClassStat::EControl::ExpandObject };
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
	return DcOk();
}

}	// namespace DcJsonHandlers

