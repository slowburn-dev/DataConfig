#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "UObject/Package.h"

FORCEINLINE_DEBUGGABLE FDcResult TryReadObjectReference(FDcDeserializeContext& Ctx, FObjectPropertyBase* ObjectProperty, UObject*& OutObject)
{
	check(ObjectProperty);

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

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

			return DC_FAIL(DcDSerDe, UObjectByStrNotFound)
				<< ObjectProperty->PropertyClass->GetFName()
				<< Value;
		}
		else
		{
			return DcSerDeUtils::TryStaticLocateObject(
				ObjectProperty->PropertyClass,
				*Value,
				OutObject);
		}
	}
	else if (Next == EDcDataEntry::MapRoot)
	{
		//	{
		//		"$type" : "FooType",
		//		"$path" : "/Game/Path/To/Object",
		//	}
		//
		//	note that this is ordered and type and path needs to be first 2 items

		DC_TRY(Ctx.Reader->ReadMapRoot());
		FString MetaKey;
		DC_TRY(Ctx.Reader->ReadString(&MetaKey));
		DC_TRY(DcSerDeUtils::ExpectMetaKey(MetaKey, TEXT("$type")));

		FString LoadClassName;
		DC_TRY(Ctx.Reader->ReadString(&LoadClassName));

		DC_TRY(Ctx.Reader->ReadString(&MetaKey));
		DC_TRY(DcSerDeUtils::ExpectMetaKey(MetaKey, TEXT("$path")));

		FString LoadPath;
		DC_TRY(Ctx.Reader->ReadString(&LoadPath));
		DC_TRY(Ctx.Reader->ReadMapEnd());

		UClass* LoadClass;
		DC_TRY(DcSerDeUtils::TryFindFirstObject<UClass>(*LoadClassName, true, LoadClass));

		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(LoadClass, ObjectProperty->PropertyClass));
		
		UObject* Loaded;
		DC_TRY(DcSerDeUtils::TryStaticLoadObject(LoadClass, nullptr, *LoadPath, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
	else if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());

		OutObject = nullptr;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch3)
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::String << Next;
	}
}

using FDcObjectReaderSignature = FDcResult(*)(FDcDeserializeContext&, FObjectPropertyBase*, UObject*&);

template<FDcObjectReaderSignature FuncObjectReader>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeObjectReference(FDcDeserializeContext& Ctx)
{
	FObjectProperty* ObjectProperty = DcPropertyUtils::CastFieldVariant<FObjectProperty>(Ctx.TopProperty());
	if (ObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	FDcClassAccess RefStat {FDcClassAccess::EControl::ReferenceOrNil };

	UObject* Loaded;
	DC_TRY(FuncObjectReader(Ctx, ObjectProperty, Loaded));

	DC_TRY(Ctx.Writer->WriteClassRootAccess(RefStat));
	if (Loaded)
	{
		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(Loaded->GetClass(), ObjectProperty->PropertyClass));
		DC_TRY(Ctx.Writer->WriteObjectReference(Loaded));
	}
	else
	{
		DC_TRY(Ctx.Writer->WriteNil());
	}
	DC_TRY(Ctx.Writer->WriteClassEndAccess(RefStat));

	return DcOk();
}

template<FDcObjectReaderSignature FuncObjectReader>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeSoftObjectReference(FDcDeserializeContext& Ctx)
{
	FSoftObjectProperty* SoftObjectProperty = DcPropertyUtils::CastFieldVariant<FSoftObjectProperty>(Ctx.TopProperty());
	if (SoftObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("SoftObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	UObject* Loaded;
	DC_TRY(FuncObjectReader(Ctx, SoftObjectProperty, Loaded));

	if (Loaded)
		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(Loaded->GetClass(), SoftObjectProperty->PropertyClass));

	DC_TRY(Ctx.Writer->WriteSoftObjectReference(FSoftObjectPtr(Loaded)));
	return DcOk();
}


template<FDcObjectReaderSignature FuncObjectReader>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeWeakObjectReference(FDcDeserializeContext& Ctx)
{
	FWeakObjectProperty* WeakObjectProperty = DcPropertyUtils::CastFieldVariant<FWeakObjectProperty>(Ctx.TopProperty());
	if (WeakObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("WeakObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	UObject* Loaded;
	DC_TRY(FuncObjectReader(Ctx, WeakObjectProperty, Loaded));

	if (Loaded)
		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(Loaded->GetClass(), WeakObjectProperty->PropertyClass));

	DC_TRY(Ctx.Writer->WriteWeakObjectReference(Loaded));
	return DcOk();
}

template<FDcObjectReaderSignature FuncObjectReader>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeLazyObjectReference(FDcDeserializeContext& Ctx)
{
	FLazyObjectProperty* LazyObjectProperty = DcPropertyUtils::CastFieldVariant<FLazyObjectProperty>(Ctx.TopProperty());
	if (LazyObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("LazyObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	UObject* Loaded;
	DC_TRY(FuncObjectReader(Ctx, LazyObjectProperty, Loaded));

	if (Loaded)
		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(Loaded->GetClass(), LazyObjectProperty->PropertyClass));

	FLazyObjectPtr LazyPtr{Loaded};
	DC_TRY(Ctx.Writer->WriteLazyObjectReference(LazyPtr));
	return DcOk();
}

template<FDcObjectReaderSignature FuncObjectReader>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeClassReference(FDcDeserializeContext& Ctx)
{
	FClassProperty* ClassProperty = DcPropertyUtils::CastFieldVariant<FClassProperty>(Ctx.TopProperty());
	if (ClassProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ClassProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	UObject* Loaded;
	DC_TRY(FuncObjectReader(Ctx, ClassProperty, Loaded));

	if (!Loaded)
	{
		DC_TRY(Ctx.Writer->WriteClassReference(nullptr));
		return DcOk();
	}
	
	if (!Loaded->IsA<UClass>())
	{
		return DC_FAIL(DcDSerDe, UObjectTypeMismatch)
			<< TEXT("UClass") << Loaded->GetClass()->GetFName();
	}

	UClass* LoadClass = CastChecked<UClass>(Loaded);
	DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(LoadClass, ClassProperty->MetaClass));

	DC_TRY(Ctx.Writer->WriteClassReference(LoadClass));
	return DcOk();
}

template<FDcObjectReaderSignature FuncObjectReader>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeSoftClassReference(FDcDeserializeContext& Ctx)
{
	FSoftClassProperty* SoftClassProperty = DcPropertyUtils::CastFieldVariant<FSoftClassProperty>(Ctx.TopProperty());
	if (SoftClassProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("SoftClassProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	UObject* Loaded;
	DC_TRY(FuncObjectReader(Ctx, SoftClassProperty, Loaded));

	if (!Loaded)
	{
		FSoftClassPath NullPath;
		DC_TRY(Ctx.Writer->WriteSoftClassReference(FSoftObjectPtr(NullPath)));
		return DcOk();
	}
	
	if (!Loaded->IsA<UClass>())
	{
		return DC_FAIL(DcDSerDe, UObjectTypeMismatch)
			<< TEXT("UClass") << Loaded->GetClass()->GetFName();
	}

	UClass* LoadClass = CastChecked<UClass>(Loaded);
	DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(LoadClass, SoftClassProperty->MetaClass));

	DC_TRY(Ctx.Writer->WriteSoftClassReference(FSoftObjectPtr(LoadClass)));
	return DcOk();
}


FORCEINLINE_DEBUGGABLE FDcResult TryReadTypeStr(FDcDeserializeContext& Ctx, FObjectPropertyBase* ObjectProperty, const FString& TypeStr, UClass*& OutClass)
{
	UObject* Obj = nullptr;
	DC_TRY(DcSerDeUtils::TryStaticLocateObject(UClass::StaticClass(), *TypeStr, Obj));
	OutClass = Cast<UClass>(Obj);
	return DcOk();
}

using FDcInlineObjectFindType = FDcResult(*)(FDcDeserializeContext&, FObjectPropertyBase*, const FString&, UClass*&);

template<FDcInlineObjectFindType FuncTypeStrLocator>
FORCEINLINE_DEBUGGABLE FDcResult DcDeserializeInstancedSubObject(FDcDeserializeContext& Ctx)
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
		return DC_FAIL(DcDSerDe, ObjectPropertyNotInline)
			<< ObjectProperty->GetFName() << ObjectProperty->GetClass()->GetFName();
	}

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		FDcClassAccess NilStat;
		NilStat.Control = FDcClassAccess::EControl::ReferenceOrNil;
		DC_TRY(Ctx.Writer->WriteClassRootAccess(NilStat));
		DC_TRY(Ctx.Writer->WriteNil());
		DC_TRY(Ctx.Writer->WriteClassEndAccess(NilStat));

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

		DC_TRY(FuncTypeStrLocator(Ctx, ObjectProperty, TypeStr, SubClassType));

		if (!SubClassType)
		{
			return DC_FAIL(DcDSerDe, UObjectByStrNotFound)
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

	DC_TRY(DcSerDeUtils::ExpectNonAbstract(SubClassType));
	DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(SubClassType, ObjectProperty->PropertyClass));

	//	construct the item
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FObjectProperty::StaticClass(), Datum));

	FObjectProperty* SubObjectProperty = Datum.CastField<FObjectProperty>();
	if (!SubObjectProperty)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Datum.Property.GetFName() << Datum.Property.GetClassName();

	if (Ctx.Objects.Num() == 0)
		return DC_FAIL(DcDSerDe, ContextMissingTopObject);

	//	don't read *Datum.DataPtr as it might be uninitialized, handle reload elsewhere
	UObject* SubObject = NewObject<UObject>(Ctx.TopObject(), SubClassType);

	ObjectProperty->SetPropertyValue(Datum.DataPtr, SubObject);
	//	manually setting class state
	DC_TRY(Ctx.Writer->PushTopClassPropertyState(Datum));

	FDcClassAccess WriteClassStat{ FDcClassAccess::EControl::ExpandObject };
	DC_TRY(Ctx.Writer->WriteClassRootAccess(WriteClassStat));

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
			return DC_FAIL(DcDSerDe, DataEntryMismatch2)
				<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
		}

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));

		DC_TRY(PutbackReader.PeekRead(&CurPeek));
	}

	DC_TRY(PutbackReader.ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteClassEndAccess(WriteClassStat));

	check(PutbackReader.Cached.Num() == 0);
	return DcOk();
}

