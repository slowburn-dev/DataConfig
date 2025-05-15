#include "DataConfig/EngineExtra/SerDe/DcSerDeBlueprint.h"
#include "DataConfig/EngineExtra/Diagnostic/DcDiagnosticEngineExtra.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Extra/SerDe/DcSerDeAnyStruct.h"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"
#include "DataConfig/Extra/Types/DcPropertyPathAccess.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"
#include "DataConfig/SerDe/DcSerializeCommon.inl"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Serialize/Handlers/Common/DcCommonSerializers.h"
#include "DataConfig/Deserialize/Handlers/Common/DcObjectDeserializers.h"

#include "Engine/Engine.h"
#include "Engine/Blueprint.h"
#include "Engine/UserDefinedStruct.h"
#include "Engine/UserDefinedEnum.h"
#include "Misc/ScopeExit.h"
#include "Misc/EngineVersionComparison.h"
#if WITH_EDITOR
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#endif // WITH_EDITOR

namespace DcEngineExtra {

static FDcResult _TryUnwrapClassObject(UObject* InObj, UClass*& OutClass)
{
	check(InObj);
	if (UBlueprint* BlueprintObj = Cast<UBlueprint>(InObj))
	{
		OutClass = BlueprintObj->GeneratedClass;
		return DcOk();
	}
	else if (UClass* ClassObj = Cast<UClass>(InObj))
	{
		OutClass = ClassObj;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDEngineExtra, ExpectBlueprintOrClass)
			<< InObj->GetClass()->GetFName();
	}
}

static FString _GetAssetPathName(UObject* Value)
{
#if UE_VERSION_OLDER_THAN(4, 26, 0)
	return Value->GetOutermost()->GetPathName();
#else
	return Value->GetPackage()->GetPathName();
#endif
}

static bool _IsAsset(UObject* Value)
{
#if UE_VERSION_OLDER_THAN(4, 27, 0)
	return Value->IsA<UBlueprintGeneratedClass>()
		|| Value->IsAsset();
#else
	return Value->IsAsset();
#endif
}

static FDcResult TryReadObjectReferenceWithBP(FDcDeserializeContext& Ctx, FProperty* Property, UClass* PropertyClass, UObject*& OutObject)
{
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
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			UObject* Loaded = nullptr;
#else
			TObjectPtr<UObject> Loaded = nullptr;
#endif

			const TCHAR* ValueBuffer = *Value;
			if (FObjectPropertyBase::ParseObjectPropertyValue(
				Property,
				nullptr,	// see PropertyHandleImpl.cpp it's using nullptr
				PropertyClass,
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
				<< PropertyClass->GetFName()
				<< Value;
		}
		else
		{
			return DcSerDeUtils::TryStaticLocateObject(
				PropertyClass,
				Value,
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
		UObject* Obj;
		DC_TRY(DcSerDeUtils::TryStaticLocateObject(UObject::StaticClass(), LoadClassName, Obj));
		DC_TRY(_TryUnwrapClassObject(Obj, LoadClass));

		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(LoadClass, PropertyClass));

		UObject* Loaded;
		DC_TRY(DcSerDeUtils::TryStaticLoadObject(LoadClass, nullptr, *LoadPath, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
	else if (Next == EDcDataEntry::None)
	{
		DC_TRY(Ctx.Reader->ReadNone());

		OutObject = nullptr;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch3)
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::None << Next;
	}
}

static FDcResult TryReadObjectReferenceWithBP(FDcDeserializeContext& Ctx, FObjectPropertyBase* ObjectProperty, UObject*& OutObject)
{
	check(ObjectProperty);
	return TryReadObjectReferenceWithBP(Ctx, ObjectProperty, ObjectProperty->PropertyClass, OutObject);
}

static FDcResult TryReadClassReferenceWithBlueprint(FDcDeserializeContext& Ctx, FObjectPropertyBase* ObjectProperty, UObject*& OutObject)
{
	//	when loading a Blueprint, unwrap to its generated class
	check(ObjectProperty);
	if (ObjectProperty->PropertyClass != UClass::StaticClass())
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("Class Property") << ObjectProperty->GetFName() << ObjectProperty->PropertyClass->GetFName();

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
			//	UE copied reference style
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			UObject* Loaded = nullptr;
#else
			TObjectPtr<UObject> Loaded = nullptr;
#endif


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
					UClass* Class;
					DC_TRY(_TryUnwrapClassObject(Loaded, Class));
					OutObject = Class;
					return DcOk();
				}
			}

			return DC_FAIL(DcDSerDe, UObjectByStrNotFound)
				<< ObjectProperty->PropertyClass->GetFName()
				<< Value;
		}
		else
		{

			UObject* Loaded;
			DC_TRY(DcSerDeUtils::TryStaticLocateObject(
				UObject::StaticClass(),
				Value,
				Loaded));

			UClass* Class;
			DC_TRY(_TryUnwrapClassObject(Loaded, Class));
			OutObject = Class;
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
		UObject* Obj;
		DC_TRY(DcSerDeUtils::TryStaticLocateObject(UObject::StaticClass(), LoadClassName, Obj));
		DC_TRY(_TryUnwrapClassObject(Obj, LoadClass));

		DC_TRY(DcSerDeUtils::ExpectLhsChildOfRhs(LoadClass, ObjectProperty->PropertyClass));

		UObject* Loaded;
		DC_TRY(DcSerDeUtils::TryStaticLoadObject(LoadClass, nullptr, *LoadPath, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
	else if (Next == EDcDataEntry::None)
	{
		DC_TRY(Ctx.Reader->ReadNone());

		OutObject = nullptr;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch3)
			<< EDcDataEntry::MapRoot << EDcDataEntry::String << EDcDataEntry::String << Next;
	}
}

FDcResult HandlerBPClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeClassReference<TryReadClassReferenceWithBlueprint>(Ctx);
}

FDcResult HandlerBPEnumDeserialize(FDcDeserializeContext& Ctx)
{
	FFieldVariant& TopProperty = Ctx.TopProperty();
	UEnum* Enum;
	FNumericProperty* UnderlyingProperty;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(Ctx.TopProperty(), Enum, UnderlyingProperty);

	if (!bIsEnum)
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
			<< TEXT("EnumProperty")  << TEXT("<NumericProperty with Enum>")
			<< TopProperty.GetFName() << TopProperty.GetClassName();

	if (Enum == nullptr)
	{
		//	fallback to read numeric
		FDcEnumData EnumData;
		DC_TRY(Ctx.Reader->ReadUInt64(&EnumData.Unsigned64));
		DC_TRY(Ctx.Writer->WriteEnum(EnumData));

		return DcOk();
	}

	UUserDefinedEnum* BPEnum = Cast<UUserDefinedEnum>(Enum);
	auto _FindBPEnumBranch = [BPEnum](const FString Str) -> FName
	{
		check(BPEnum);
		FText StrText = FText::FromString(Str);
		for (auto& Pair : BPEnum->DisplayNameMap)
		{
			if (Pair.Value.EqualTo(StrText))
				return Pair.Key;
		}

		return FName(NAME_None);
	};

	bool bIsBitFlags;

#if WITH_EDITORONLY_DATA
#if WITH_METADATA
	bIsBitFlags = Enum->HasMetaData(TEXT("Bitflags"));
#else
	//	Program target is missing `UEnum::HasMetaData`
	bIsBitFlags = ((UField*)Enum)->HasMetaData(TEXT("Bitflags"));
#endif // WITH_METADATA
#else
	bIsBitFlags = false;
#endif // WITH_EDITORONLY_DATA

	if (!bIsBitFlags)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

		FName ValueName{NoInit};
		if (BPEnum)
			ValueName = FName(BPEnum->GenerateFullEnumName(*_FindBPEnumBranch(Value).ToString()));
		else
			ValueName = FName(Enum->GenerateFullEnumName(*Value));

		if (!Enum->IsValidEnumName(ValueName))
			return DC_FAIL(DcDReadWrite, EnumNameNotFound) << Enum->GetFName() << Value;

		FDcEnumData EnumData;
		EnumData.Signed64 = Enum->GetValueByName(ValueName);

		DC_TRY(Ctx.Writer->WriteEnum(EnumData));
		return DcOk();
	}
	else
	{
		FDcEnumData EnumData;
		EnumData.Signed64 = 0;

		DC_TRY(Ctx.Reader->ReadArrayRoot());
		while (true)
		{
			EDcDataEntry Next;
			DC_TRY(Ctx.Reader->PeekRead(&Next));
			if (Next == EDcDataEntry::ArrayEnd)
				break;

			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));

			FName ValueName{NoInit};
			if (BPEnum)
				ValueName = FName(BPEnum->GenerateFullEnumName(*_FindBPEnumBranch(Value).ToString()));
			else
				ValueName = FName(Enum->GenerateFullEnumName(*Value));

			if (!Enum->IsValidEnumName(ValueName))
				return DC_FAIL(DcDReadWrite, EnumNameNotFound) << Enum->GetFName() << Value;

			EnumData.Signed64 |= Enum->GetValueByName(ValueName);
		}
		DC_TRY(Ctx.Reader->ReadArrayEnd());
		DC_TRY(Ctx.Writer->WriteEnum(EnumData));

		return DcOk();
	}
}
static FDcResult TryWriteObjectReferenceWithBP(FDcSerializeContext& Ctx, FObjectPropertyBase* ObjectProperty, UObject* Value)
{
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value));
	if (Value == nullptr)
		DC_TRY(Ctx.Writer->WriteNone());
	else if (_IsAsset(Value))
		DC_TRY(Ctx.Writer->WriteString(_GetAssetPathName(Value)));
	else
		DC_TRY(Ctx.Writer->WriteString(DcSerDeUtils::FormatObjectName(Value)));

	return DcOk();
}

FDcResult HandlerBPClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeClassReference<&TryWriteObjectReferenceWithBP>(Ctx);
}

FDcResult HandlerBPObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeObjectReference<&TryWriteObjectReferenceWithBP>(Ctx);
}

FDcResult HandlerBPObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeObjectReference<&TryReadObjectReferenceWithBP>(Ctx);
}

FDcResult HandlerBPEnumSerialize(FDcSerializeContext& Ctx)
{
	FFieldVariant& TopProperty = Ctx.TopProperty();
	UEnum* Enum;
	FNumericProperty* UnderlyingProperty;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(TopProperty, Enum, UnderlyingProperty);
	if (!bIsEnum)
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
			<< TEXT("EnumProperty")  << TEXT("<NumericProperty with Enum>")
			<< TopProperty.GetFName() << TopProperty.GetClassName();

	FDcEnumData Value;
	DC_TRY(Ctx.Reader->ReadEnum(&Value));

	if (Enum == nullptr)
	{
		//	fallback to write numeric
		return Ctx.Writer->WriteUInt64(Value.Unsigned64);
	}


	bool bIsBitFlags;
#if WITH_EDITORONLY_DATA
#if WITH_METADATA
	bIsBitFlags = Enum->HasMetaData(TEXT("Bitflags"));
#else
	//	Program target is missing `UEnum::HasMetaData`
	bIsBitFlags = ((UField*)Enum)->HasMetaData(TEXT("Bitflags"));
#endif // WITH_METADATA
#else
	bIsBitFlags = false;
#endif // WITH_EDITORONLY_DATA

	UUserDefinedEnum* BPEnum = Cast<UUserDefinedEnum>(Enum);
	if (!bIsBitFlags)
	{
		int ValueIndex = Enum->GetIndexByValue(Value.Signed64);
		if (ValueIndex == INDEX_NONE)
			return DC_FAIL(DcDReadWrite, EnumValueInvalid)
				<< Enum->GetName() << Value.Signed64;

		if (BPEnum)
		{
			int Index = BPEnum->GetIndexByValue(Value.Signed64);
			DC_TRY(Ctx.Writer->WriteString(BPEnum->GetAuthoredNameStringByIndex(Index)));
		}
		else
		{
			DC_TRY(Ctx.Writer->WriteString(Enum->GetNameStringByValue(Value.Signed64)));
		}
	}
	else
	{
		DC_TRY(Ctx.Writer->WriteArrayRoot());

		int Len = Enum->NumEnums();
		uint64 Data = Value.Unsigned64;
		for (int Ix = 0; Ix < Len; Ix++)
		{
			int64 Cur = Enum->GetValueByIndex(Ix);
			int CurPopCount = FPlatformMath::CountBits(Cur);
			if (CurPopCount == 0)
				continue;

			if (FPlatformMath::CountBits(Data & Cur) == CurPopCount)
			{
				DC_TRY(Ctx.Writer->WriteString(Enum->GetNameStringByIndex(Ix)));
				Data ^= Cur;
			}
		}

		if (Data != 0)
			return DC_FAIL(DcDSerDe, EnumBitFlagsNotFullyMasked)
				<< Enum->GetName();

		DC_TRY(Ctx.Writer->WriteArrayEnd());
	}

	return DcOk();
}

FDcResult HandlerBPStructSerialize(FDcSerializeContext& Ctx)
{
#if WITH_EDITOR
	UStruct* Struct = DcPropertyUtils::TryGetStruct(Ctx.TopProperty());
	if (UUserDefinedStruct* BPStruct = Cast<UUserDefinedStruct>(Struct))
	{
		auto _FindShortPropertyName = [BPStruct](FName FullName)
		{
			FStructVariableDescription* VarDesc = CastChecked<UUserDefinedStructEditorData>(BPStruct->EditorData)->VariablesDescriptions.FindByPredicate(
			[&FullName](FStructVariableDescription& Cur){
				return Cur.VarName == FullName;
			});
			return VarDesc ? FName(VarDesc->FriendlyName) : FName(NAME_None);
		};

		DC_TRY(Ctx.Reader->ReadStructRoot());
		DC_TRY(Ctx.Writer->WriteMapRoot());

		EDcDataEntry CurPeek;
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&CurPeek));

			if (CurPeek == EDcDataEntry::StructEnd)
			{
				break;
			}
			else if (CurPeek == EDcDataEntry::Name)
			{
				FName Value;
				DC_TRY(Ctx.Reader->ReadName(&Value));
				DC_TRY(Ctx.Writer->WriteName(_FindShortPropertyName(Value)));
			}
			else
			{
				return DC_FAIL(DcDSerDe, DataEntryMismatch)
					<< EDcDataEntry::Name << CurPeek;
			}

			DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
		}

		DC_TRY(Ctx.Reader->ReadStructEnd());
		DC_TRY(Ctx.Writer->WriteMapEnd());

		return DcOk();
	}
	else
	{
		return DcCommonHandlers::HandlerStructToMapSerialize(Ctx);
	}
#else
	return DcCommonHandlers::HandlerStructToMapSerialize(Ctx);
#endif // WITH_EDITOR
}

FDcResult HandlerBPDcAnyStructSerialize(FDcSerializeContext& Ctx)
{
	return DcExtra::DcHandlerSerializeAnyStruct(Ctx, [](UScriptStruct* Struct)
	{
		if (Struct->IsAsset())
			return _GetAssetPathName(Struct);
		else
			return DcSerDeUtils::FormatObjectName(Struct);
	});
}

FDcResult HandlerBPStructRootSerializeSkipEmptyAnyStruct(FDcSerializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadStructRoot());
	DC_TRY(Ctx.Writer->WriteMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));

		if (CurPeek == EDcDataEntry::StructEnd)
			break;

		if (CurPeek == EDcDataEntry::Name)
		{

			FName Value;
			DC_TRY(Ctx.Reader->ReadName(&Value));

			FFieldVariant CurProperty;
			DC_TRY(Ctx.Reader->PeekReadProperty(&CurProperty));
			UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(CurProperty);
			if (Struct == FDcAnyStruct::StaticStruct())
			{
				void* DataPtr;
				DC_TRY(Ctx.Reader->PeekReadDataPtr(&DataPtr));
				FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)DataPtr;

				if (!AnyStructPtr->IsValid())
				{
					DC_TRY(Ctx.Reader->SkipRead());
				}
				else
				{
					DC_TRY(Ctx.Writer->WriteName(Value));
					DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
				}
			}
			else
			{
				DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
			}
		}
		else
		{
			return DC_FAIL(DcDSerDe, DataEntryMismatch)
				<< EDcDataEntry::Name << CurPeek;
		}
	}

	DC_TRY(Ctx.Reader->ReadStructEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

} // namespace DcEngineExtra

FString UDcTestBPClassBase::GetClassID_Implementation()
{
	return TEXT("UBPClassBase");
}

static void _SetupTestBPJsonDeserializeHandlers(FDcDeserializer& Deserializer)
{
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize));

	Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcEngineExtra::HandlerBPClassReferenceDeserialize));
	Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerObjectReferenceDeserialize));
}

///	Note that these test depends on '.uasset` that comes with the plugin
DC_TEST("DataConfig.EngineExtra.BPClassReference")
{
	GEngine->Exec(nullptr, TEXT("log LogLinker off"));
	GEngine->Exec(nullptr, TEXT("log LogUObjectGlobals off"));
	ON_SCOPE_EXIT
	{
		GEngine->Exec(nullptr, TEXT("log LogLinker on"));
		GEngine->Exec(nullptr, TEXT("log LogUObjectGlobals on"));
	};

	FDcEngineExtraTestStructWithBPClass Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(
		{
			"ClassField1" : null,
			"ClassField2" : "DcTestNativeDerived1",
			"ClassField3" : "/DataConfig/DcFixture/DcTestBlueprintClassBeta"
		}
	)");

	FDcJsonReader Reader(Str);
	{
		UTEST_OK("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			_SetupTestBPJsonDeserializeHandlers(*Ctx.Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		UTEST_TRUE("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Dest.ClassField1 == nullptr);
		UTEST_TRUE("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Dest.ClassField2 == UDcTestNativeDerived1::StaticClass());
		UTEST_TRUE("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Dest.ClassField3->GetFName() == TEXT("DcTestBlueprintClassBeta_C"));
	}

	{
		FString StrBad = TEXT(R"(
			{
				"ClassField1" : "/DataConfig/Path/To/Nowhere"
			}
		)");
		UTEST_OK("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Reader.SetNewString(*StrBad));
		UTEST_DIAG("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			_SetupTestBPJsonDeserializeHandlers(*Ctx.Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing),
			DcDSerDe, UObjectByStrNotFound);
	}

	{
		FString StrBad = TEXT(R"(
			{
				"ClassField1" : "/DataConfig/DcFixture/DcTestBlueprintInstanceAlpha"
			}
		)");
		Reader.AbortAndUninitialize();	// need abort due to previous error

		UTEST_OK("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Reader.SetNewString(*StrBad));
		UTEST_DIAG("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			_SetupTestBPJsonDeserializeHandlers(*Ctx.Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing),
			DcDEngineExtra, ExpectBlueprintOrClass);
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->FieldClassSerializerMap[FClassProperty::StaticClass()] = FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPClassReferenceSerialize);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}

DC_TEST("DataConfig.EngineExtra.BPObjectInstance")
{
	FDcEngineExtraTestStructWithBPInstance Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(
		{
			"InstanceField1" : null,
			"InstanceField2" : "/DataConfig/DcFixture/DcTestBlueprintInstanceAlpha",
			"InstanceField3" : "/DataConfig/DcFixture/DcTestBlueprintInstanceBeta"
		}
	)");

	{
		FDcJsonReader Reader(Str);

		UTEST_OK("Editor Extra FDcEngineExtraTestStructWithBPInstance SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->FieldClassDeserializerMap[FObjectProperty::StaticClass()] = FDcDeserializeDelegate::CreateStatic(DcEngineExtra::HandlerBPObjectReferenceDeserialize);
		}));
		UTEST_TRUE("Editor Extra FDcEngineExtraTestStructWithBPInstance SerDe", Dest.InstanceField1 == nullptr);
		//	note that `GetClassID` calling into BP method
		UTEST_TRUE("Editor Extra FDcEngineExtraTestStructWithBPInstance SerDe", Dest.InstanceField2->GetClassID() == TEXT("DooFoo253"));
		UTEST_TRUE("Editor Extra FDcEngineExtraTestStructWithBPInstance SerDe", Dest.InstanceField3->GetClassID() == TEXT("BetaNottrue"));
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Editor Extra FDcEngineExtraTestStructWithBPInstance SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->FieldClassSerializerMap[FObjectProperty::StaticClass()] = FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPObjectReferenceSerialize);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Editor Extra FDcEngineExtraTestStructWithBPClass SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}

DC_TEST("DataConfig.EngineExtra.BPClassInstance")
{
	FString Str = TEXT(R"(

		{
			"StrField" : "Foo",
			"BPEnumField" : "Baz",
			"IntField" : 345
		}

	)");
	FDcJsonReader Reader(Str);

	UBlueprint* Blueprint;
	UTEST_OK("Extra BPClassInstance SerDe",
		DcSerDeUtils::TryLoadObject(nullptr, TEXT("/DataConfig/DcFixture/DcTestBlueprintClassAlpha"), Blueprint)
	);


#if UE_VERSION_OLDER_THAN(4, 26, 0)
	UObject* Obj = StaticConstructObject_Internal(
		Blueprint->GeneratedClass
	);
#else
	FStaticConstructObjectParameters Params(Blueprint->GeneratedClass);
	Params.Outer = GetTransientPackage();
	Params.Name = NAME_None;
	Params.SetFlags = RF_NoFlags;
	Params.Template = nullptr;
	Params.bCopyTransientsFromClassDefaults = false;
	Params.InstanceGraph = nullptr;
	Params.ExternalPackage = nullptr;
	UObject* Obj = StaticConstructObject_Internal(Params);
#endif

	FDcPropertyDatum DestDatum(Obj);

	{
		UTEST_OK("Extra BPClassInstance SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddDirectHandler(
				UBlueprintGeneratedClass::StaticClass(),
				FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToClassDeserialize)
			);
			Ctx.Deserializer->PredicatedDeserializers.FindByPredicate([](auto& Entry){
				return Entry.Name == FName(TEXT("Enum"));
			})->Handler = FDcDeserializeDelegate::CreateStatic(DcEngineExtra::HandlerBPEnumDeserialize);
		}, DcAutomationUtils::EDefaultSetupType::SetupJSONHandlers));

		UTEST_TRUE("Extra BPClassInstance SerDe", DcAutomationUtils::DebugGetScalarPropertyValue<FString>(DestDatum, TEXT("StrField")) == TEXT("Foo"));
		UTEST_TRUE("Extra BPClassInstance SerDe", DcAutomationUtils::DebugGetEnumPropertyIndex(DestDatum, TEXT("BPEnumField")) == 2);
		UTEST_TRUE("Extra BPClassInstance SerDe", DcAutomationUtils::DebugGetScalarPropertyValue<int32>(DestDatum, TEXT("IntField")) == 345);

		UDcTestBPClassBase* BPBase = Cast<UDcTestBPClassBase>(Obj);
		UTEST_NOT_NULL("Extra BPClassInstance SerDe", BPBase);
		UTEST_EQUAL("Extra BPClassInstance SerDe", BPBase->GetClassID(), TEXT("FooBaz345"));
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Extra BPClassInstance SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			//	skip `NativeClass` field
			auto OldPredicate = Ctx.Reader->Config.ProcessPropertyPredicate;
			Ctx.Reader->Config.ProcessPropertyPredicate = FDcProcessPropertyPredicateDelegate::CreateLambda(
			[OldPredicate = MoveTemp(OldPredicate)](FProperty* Property)
			{
				if (Property->GetFName() == TEXT("NativeClass")
					&& Property->IsA<FClassProperty>())
				{
					return false;
				}
				else
				{
					check(OldPredicate.IsBound());
					return OldPredicate.Execute(Property);
				}
			});

			Ctx.Serializer->AddDirectHandler(
				UBlueprintGeneratedClass::StaticClass(),
				FDcSerializeDelegate::CreateStatic(DcCommonHandlers::HandlerClassToMapSerialize)
			);
			Ctx.Serializer->PredicatedSerializers.FindByPredicate([](auto& Entry){
				return Entry.Name == FName(TEXT("Enum"));
			})->Handler = FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPEnumSerialize);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Extra BPClassInstance SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}

#if WITH_EDITOR
DC_TEST("DataConfig.EngineExtra.BPStructInstance")
{
	using namespace DcExtra;
	FDcExtraTestWithAnyStruct1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(

		{
			"AnyStructField1" : {
				"$type" : "/DataConfig/DcFixture/DcTestBlueprintStructWithColor",
				"NameField" : "Foo",
				"StrField" : "Bar",
				"IntField" : 123,
				"ColorField" : "#FF0000FF"
			}
		}

	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_OK("Extra BPStruct FAnyStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
			[](FDcDeserializeContext& Ctx) {
				Ctx.Deserializer->AddStructHandler(
					FDcAnyStruct::StaticStruct(),
					FDcDeserializeDelegate::CreateStatic(HandlerDcAnyStructDeserialize)
				);
				Ctx.Deserializer->AddStructHandler(
					TBaseStructure<FColor>::Get(),
					FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
				);
				Ctx.Deserializer->AddDirectHandler(
					UUserDefinedStruct::StaticClass(),
					FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize)
				);
			}));


		FDcPropertyDatum Field1Datum(Dest.AnyStructField1.StructClass, Dest.AnyStructField1.DataPtr);

		UTEST_TRUE("Extra BPStruct FAnyStruct SerDe", DcAutomationUtils::DebugGetScalarPropertyValue<FName>(Field1Datum, TEXT("NameField")) == FName(TEXT("Foo")));
		UTEST_TRUE("Extra BPStruct FAnyStruct SerDe", DcAutomationUtils::DebugGetScalarPropertyValue<FString>(Field1Datum, TEXT("StrField")) == FString(TEXT("Bar")));
		UTEST_TRUE("Extra BPStruct FAnyStruct SerDe", DcAutomationUtils::DebugGetScalarPropertyValue<int32>(Field1Datum, TEXT("IntField")) == 123);

		FDcPropertyDatum ColorDatum;
		UTEST_OK("Extra BPStruct FAnyStruct SerDe", DcExtra::GetDatumPropertyByPath(Field1Datum, TEXT("ColorField"), ColorDatum));
		UTEST_TRUE("Extra BPStruct FAnyStruct SerDe", *((FColor*)ColorDatum.DataPtr) == FColor::Red);
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Extra BPStruct FAnyStruct SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->FieldClassSerializerMap[FObjectProperty::StaticClass()] = FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPStructRootSerializeSkipEmptyAnyStruct);
			Ctx.Serializer->UClassSerializerMap[UScriptStruct::StaticClass()] = FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPStructRootSerializeSkipEmptyAnyStruct);

			Ctx.Serializer->AddStructHandler(
				FDcAnyStruct::StaticStruct(),
				FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPDcAnyStructSerialize)
			);
			Ctx.Serializer->AddStructHandler(
				TBaseStructure<FColor>::Get(),
				FDcSerializeDelegate::CreateStatic(HandlerColorSerialize)
			);
			Ctx.Serializer->AddDirectHandler(
				UUserDefinedStruct::StaticClass(),
				FDcSerializeDelegate::CreateStatic(DcEngineExtra::HandlerBPStructSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Extra BPStruct FAnyStruct SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}
#endif // WITH_EDITOR
