#include "DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Deserialize/Handlers/Json/DcJsonStructDeserializers.h"
#include "DataConfig/Deserialize/Handlers/Json/DcJsonClassDeserializers.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/Extra/Types/DcPropertyPathAccess.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"

#include "Engine/Blueprint.h"
#include "Engine/UserDefinedStruct.h"
#include "Misc/ScopeExit.h"

namespace DcEditorExtra {

FDcResult HandlerBPClassReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::String
		|| Next == EDcDataEntry::Nil;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::ClassReference, &bWritePass));

	bool bPropertyPass = Ctx.TopProperty().IsA<FClassProperty>();
	if (!(bRootPeekPass && bWritePass && bPropertyPass))
		return DcOkWithFallThrough(OutRet);

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		DC_TRY(Ctx.Writer->WriteClassReference(nullptr));
	}
	else if (Next == EDcDataEntry::String)
	{
		FString ClassStr;
		DC_TRY(Ctx.Reader->ReadString(&ClassStr));

		UClass* LoadClass = nullptr;
		if (ClassStr.StartsWith("/"))
		{
			UObject* Loaded = StaticLoadObject(UBlueprint::StaticClass(), nullptr, *ClassStr, nullptr);
			if (Loaded == nullptr)
				return DC_FAIL(DcDEditorExtra, LoadObjectByPathFail) << TEXT("Blueprint") << ClassStr;
			LoadClass = CastChecked<UBlueprint>(Loaded)->GeneratedClass;
		}
		else
		{
			LoadClass = FindObject<UClass>(ANY_PACKAGE, *ClassStr, true);
			if (LoadClass == nullptr)
				return DC_FAIL(DcDDeserialize, UObjectByNameNotFound) << TEXT("Class") << ClassStr;
		}
		check(LoadClass);

		FClassProperty* ClassProperty = CastFieldChecked<FClassProperty>(Ctx.TopProperty().ToFieldUnsafe());
		check(ClassProperty && ClassProperty->MetaClass);
		if (!LoadClass->IsChildOf(ClassProperty->MetaClass))
		{
			return DC_FAIL(DcDDeserialize, ClassLhsIsNotChildOfRhs)
				<< LoadClass->GetFName() << ClassProperty->MetaClass->GetFName();
		}

		DC_TRY(Ctx.Writer->WriteClassReference(LoadClass));
	}
	else
	{
		return DcNoEntry();
	}

	return DcOkWithProcessed(OutRet);
}

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::MapRoot
		|| Next == EDcDataEntry::Nil;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)Datum.DataPtr;

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		AnyStructPtr->Reset();
	}
	else
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		FString Str;
		DC_TRY(Ctx.Reader->ReadString(&Str));
		if (!DcDeserializeUtils::IsMeta(Str))
			return DC_FAIL(DcDDeserialize, ExpectMetaType);


		UScriptStruct* LoadStruct = nullptr;
		DC_TRY(Ctx.Reader->ReadString(&Str));

		if (Str.StartsWith("/"))
		{
			UObject* Loaded = StaticLoadObject(UUserDefinedStruct::StaticClass(), nullptr, *Str, nullptr);
			if (Loaded == nullptr)
				return DC_FAIL(DcDEditorExtra, LoadObjectByPathFail) << TEXT("UserDefinedStruct") << Str;

			LoadStruct = CastChecked<UScriptStruct>(Loaded);
		}
		else
		{
			LoadStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *Str, true);
			if (LoadStruct == nullptr)
				return DC_FAIL(DcDDeserialize, UObjectByNameNotFound) << TEXT("ScriptStruct") << MoveTemp(Str);
		}

		void* DataPtr = (uint8*)FMemory::Malloc(LoadStruct->GetStructureSize());
		LoadStruct->InitializeStruct(DataPtr);
		FDcAnyStruct TmpAny{ DataPtr, LoadStruct };

		*AnyStructPtr = MoveTemp(TmpAny);

		Ctx.Writer->PushTopStructPropertyState({ LoadStruct, (void*)AnyStructPtr->DataPtr }, Ctx.TopProperty().GetFName());

		FDcPutbackReader PutbackReader(Ctx.Reader);
		PutbackReader.Putback(EDcDataEntry::MapRoot);
		TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	return DcOkWithProcessed(OutRet);
}

} // namespace DcEditorExtra

FString UDcTestBPClassBase::GetClassID_Implementation()
{
	return TEXT("UBPClassBase");
}

static void _SetupTestBPJsonDeserializeHandlers(FDcDeserializer& Deserializer)
{
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcJsonHandlers::HandlerStructRootDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcJsonHandlers::HandlerStructRootDeserialize));

	Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcEditorExtra::HandlerBPClassReferenceDeserialize));
	Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcJsonHandlers::HandlerObjectReferenceDeserialize));
}

///	Note that these test depends on '.uasset` that comes with the plugin
DC_TEST("DataConfig.EditorExtra.BPClassReference")
{
	GEngine->Exec(nullptr, TEXT("log LogLinker off"));
	GEngine->Exec(nullptr, TEXT("log LogUObjectGlobals off"));
	ON_SCOPE_EXIT
	{
		GEngine->Exec(nullptr, TEXT("log LogLinker on"));
		GEngine->Exec(nullptr, TEXT("log LogUObjectGlobals on"));
	};

	FDcEditorExtraTestStructWithBPClass Dest;
	FDcPropertyDatum DestDatum(FDcEditorExtraTestStructWithBPClass::StaticStruct(), &Dest);

	FString Str = TEXT(R"(
		{
			"ClassField1" : null,
			"ClassField2" : "DcTestNativeDerived1",
			"ClassField3" : "/DataConfig/DcFixture/DcTestBlueprintClassBeta",
		}
	)");
	FDcJsonReader Reader(Str);

	UTEST_OK("Editor Extra FDcEditorExtraTestStructWithBPClass Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		_SetupTestBPJsonDeserializeHandlers(Deserializer);
	}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

	UTEST_TRUE("Editor Extra FDcEditorExtraTestStructWithBPClass Deserialize", Dest.ClassField1 == nullptr);
	UTEST_TRUE("Editor Extra FDcEditorExtraTestStructWithBPClass Deserialize", Dest.ClassField2 == UDcTestNativeDerived1::StaticClass());
	UTEST_TRUE("Editor Extra FDcEditorExtraTestStructWithBPClass Deserialize", Dest.ClassField3->GetFName() == TEXT("DcTestBlueprintClassBeta_C"));

	{
		FString StrBad = TEXT(R"(
			{
				"ClassField1" : "/DataConfig/Path/To/Nowhere"
			}
		)");
		UTEST_OK("Editor Extra FGameplayTag Deserialize", Reader.SetNewString(*StrBad));
		UTEST_DIAG("Editor Extra FDcEditorExtraTestStructWithBPClass Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			_SetupTestBPJsonDeserializeHandlers(Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing),
			DcDEditorExtra, LoadObjectByPathFail);
	}

	{
		FString StrBad = TEXT(R"(
			{
				"ClassField1" : "/DataConfig/DcFixture/DcTestBlueprintInstanceAlpha"
			}
		)");
		Reader.AbortAndUninitialize();	// need abort due to previous error

		UTEST_OK("Editor Extra FGameplayTag Deserialize", Reader.SetNewString(*StrBad));
		UTEST_DIAG("Editor Extra FDcEditorExtraTestStructWithBPClass Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			_SetupTestBPJsonDeserializeHandlers(Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing),
			DcDEditorExtra, LoadObjectByPathFail);
	}

	return true;
}

DC_TEST("DataConfig.EditorExtra.BPObjectInstance")
{
	FDcEditorExtraTestStructWithBPInstance Dest;
	FDcPropertyDatum DestDatum(FDcEditorExtraTestStructWithBPInstance::StaticStruct(), &Dest);

	FString Str = TEXT(R"(
		{
			"InstanceField1" : null,
			"InstanceField2" : "/DataConfig/DcFixture/DcTestBlueprintInstanceAlpha",
			"InstanceField3" : "/DataConfig/DcFixture/DcTestBlueprintInstanceBeta",
		}
	)");
	FDcJsonReader Reader(Str);

	UTEST_OK("Editor Extra FDcEditorExtraTestStructWithBPInstance Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));

	UTEST_TRUE("Editor Extra FDcEditorExtraTestStructWithBPInstance Deserialize", Dest.InstanceField1 == nullptr);
	//	note that `GetClassID` calling into BP method
	UTEST_TRUE("Editor Extra FDcEditorExtraTestStructWithBPInstance Deserialize", Dest.InstanceField2->GetClassID() == TEXT("DooFoo253"));
	UTEST_TRUE("Editor Extra FDcEditorExtraTestStructWithBPInstance Deserialize", Dest.InstanceField3->GetClassID() == TEXT("BetaNottrue"));

	return true;
}

DC_TEST("DataConfig.EditorExtra.BPStruct")
{
	using namespace DcExtra;
	FDcExtraTestWithAnyStruct1 Dest;
	FDcPropertyDatum DestDatum(FDcExtraTestWithAnyStruct1::StaticStruct(), &Dest);

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
	FDcJsonReader Reader(Str);

	UTEST_OK("Extra BPStruct FAnyStruct Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsDcAnyStruct),
				//	using handler that handles `UserDefinedStruct`
				FDcDeserializeDelegate::CreateStatic(DcEditorExtra::HandlerBPDcAnyStructDeserialize)
			);
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
			);
			Deserializer.AddDirectHandler(
				UUserDefinedStruct::StaticClass(),
				FDcDeserializeDelegate::CreateStatic(DcJsonHandlers::HandlerStructRootDeserialize)
			);
		}));


	FDcPropertyDatum Field1Datum(Dest.AnyStructField1.StructClass, Dest.AnyStructField1.DataPtr);

	UTEST_TRUE("Extra BPStruct FAnyStruct Deserialize", DcAutomationUtils::DebugGetScalarPropertyValue<FName>(Field1Datum, TEXT("NameField")) == FName(TEXT("Foo")));
	UTEST_TRUE("Extra BPStruct FAnyStruct Deserialize", DcAutomationUtils::DebugGetScalarPropertyValue<FString>(Field1Datum, TEXT("StrField")) == FString(TEXT("Bar")));
	UTEST_TRUE("Extra BPStruct FAnyStruct Deserialize", DcAutomationUtils::DebugGetScalarPropertyValue<int32>(Field1Datum, TEXT("IntField")) == 123);

	FDcPropertyDatum ColorDatum;
	UTEST_OK("Extra BPStruct FAnyStruct Deserialize", DcExtra::GetDatumPropertyByPath(Field1Datum, TEXT("ColorField"), ColorDatum));
	UTEST_TRUE("Extra BPStruct FAnyStruct Deserialize", *((FColor*)ColorDatum.DataPtr) == FColor::Red);

	return true;
}

