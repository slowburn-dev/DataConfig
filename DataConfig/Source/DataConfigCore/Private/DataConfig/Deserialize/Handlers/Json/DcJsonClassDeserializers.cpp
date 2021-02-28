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
			return DC_FAIL(DcDDeserialize, UObjectByNameNotFound) << TEXT("Class") << ClassStr;

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

static FDcResult LoadObjectByPath(FObjectProperty* ObjectProperty, UClass* LoadClass, FString LoadPath, FDcDeserializeContext& Ctx, UObject*& OutLoaded)
{
	DC_TRY(DcExpect(LoadClass != nullptr));
	DC_TRY(DcExpect(LoadClass->IsChildOf(ObjectProperty->PropertyClass)));
	UObject* Loaded = StaticLoadObject(LoadClass, nullptr, *LoadPath, nullptr);
	DC_TRY(DcExpect(Loaded != nullptr));

	OutLoaded = Loaded;
	return DcOk();
}

FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FObjectProperty* ObjectProperty = DcPropertyUtils::CastFieldVariant<FObjectProperty>(Ctx.TopProperty());
	if (ObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	FDcClassStat RefStat {
		ObjectProperty->PropertyClass->GetFName(), FDcClassStat::EControl::ReferenceOrNil
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

					return DcOk();
				}
			}

			return DcFail();
		}
		else if (Value.StartsWith(TEXT("/")))
		{
			//	/Game/Path/To/Object
			//	`Game` is a Mount Point, and there's no `.uasset` suffix
			UObject* Loaded = nullptr;
			DC_TRY(LoadObjectByPath(ObjectProperty, ObjectProperty->PropertyClass, Value, Ctx, Loaded));

			DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
			DC_TRY(Ctx.Writer->WriteObjectReference(Loaded));
			DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

			return DcOk();
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

		return DcOk();
	}
	else if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		DC_TRY(Ctx.Writer->WriteClassRoot(RefStat));
		DC_TRY(Ctx.Writer->WriteNil());
		DC_TRY(Ctx.Writer->WriteClassEnd(RefStat));

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch3)
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::String << Next;
	}
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
		return DcFail();
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

	FObjectProperty* SubObjectProperty = Datum.CastField<FObjectProperty>();
	if (!SubObjectProperty)
		return DcFail();

	//	don't read *Datum.DataPtr as it might be unitnialized, handle reload elsewhere
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

