#include "DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "Modules/Boilerplate/ModuleBoilerplate.h"

namespace DcExtra
{
EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx)
{
	if (FStructProperty* StructProperty = DcPropertyUtils::CastFieldVariant<FStructProperty>(Ctx.TopProperty()))
	{
		return StructProperty->Struct == FDcAnyStruct::StaticStruct()
			? EDcDeserializePredicateResult::Process
			: EDcDeserializePredicateResult::Pass;
	}
	else
	{
		return EDcDeserializePredicateResult::Pass;
	}
}

FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	DC_TRY(Ctx.Reader->ReadMapRoot());
	FString Str;
	DC_TRY(Ctx.Reader->ReadString(&Str));
	if (!DcDeserializeUtils::IsMeta(Str))
		return DC_FAIL(DcDDeserialize, ExpectMetaType);

	DC_TRY(Ctx.Reader->ReadString(&Str));
	UScriptStruct* LoadStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *Str, true);
	if (LoadStruct == nullptr)
		return DC_FAIL(DcDDeserialize, UObjectByNameNotFound) << MoveTemp(Str);

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	void* DataPtr = (uint8*)FMemory::Malloc(LoadStruct->GetStructureSize());
	LoadStruct->InitializeStruct(DataPtr);
	FDcAnyStruct TmpAny{DataPtr, LoadStruct};

	FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)Datum.DataPtr;
	*AnyStructPtr = MoveTemp(TmpAny);

	Ctx.Writer->PushTopStructPropertyState({LoadStruct, (void*)AnyStructPtr->DataPtr}, Ctx.TopProperty().GetFName());

	FDcPutbackReader PutbackReader(Ctx.Reader);
	PutbackReader.Putback(EDcDataEntry::MapRoot);
	TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

	FDcScopedProperty ScopedValueProperty(Ctx);
	DC_TRY(ScopedValueProperty.PushProperty());
	DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

	return DcOkWithProcessed(OutRet);
}

}	//	namespace DcExtra

static FDcAnyStruct _IdentityByValue(FDcAnyStruct Handle)
{
	return Handle;
}

//	override new/deletes as DcExtra is a pure source module, missing module boillerplates 
REPLACEMENT_OPERATOR_NEW_AND_DELETE

DC_TEST("DataConfig.Extra.Deserialize.AnyStructUsage")
{
	{
		FDcAnyStruct Alhpa(new FDcExtraTestSimpleStruct1());
		FDcAnyStruct Beta = Alhpa;

		UTEST_EQUAL("Extra AnyStruct usage", Alhpa.GetSharedReferenceCount(), 2);
	}

	{
		uint32 DestructCalledCount = 0;
		{
			FDcAnyStruct Any1(new FDcExtraTestDestructDelegateContainer());
			Any1.GetChecked<FDcExtraTestDestructDelegateContainer>()->DestructAction.BindLambda([&DestructCalledCount]{
				DestructCalledCount++;
			});

			FDcAnyStruct Any2{ Any1 };
			FDcAnyStruct Any3 = MoveTemp(Any1);
			FDcAnyStruct Any4 = _IdentityByValue(Any2);
		}

		UTEST_EQUAL("Extra AnyStruct usage", DestructCalledCount, 1);
	}

	return true;
}

DC_TEST("DataConfig.Extra.Deserialize.AnyStructDeserialize")
{
	using namespace DcExtra;
	FDcExtraTestWithAnyStruct1 Dest;
	FDcPropertyDatum DestDatum(FDcExtraTestWithAnyStruct1::StaticStruct(), &Dest);

	FString Str = TEXT(R"(

		{
			"AnyStructField1" : {
				"$type" : "DcExtraTestSimpleStruct1",
				"NameField" : "Foo"
			},
			"AnyStructField2" : {
				"$type" : "DcExtraTestStructWithColor1",
				"ColorField1" : "#0000FFFF",
				"ColorField2" : "#FF0000FF"
			}
		}

	)");
	FDcJsonReader Reader(Str);


	UTEST_OK("Extra FAnyStruct Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsDcAnyStruct),
			FDcDeserializeDelegate::CreateStatic(HandlerDcAnyStructDeserialize)
		);
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
			FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
		);
	}));

	UTEST_TRUE("Extra FAnyStruct Deserialize", Dest.AnyStructField1.GetChecked<FDcExtraTestSimpleStruct1>()->NameField == TEXT("Foo"));
	UTEST_TRUE("Extra FAnyStruct Deserialize", Dest.AnyStructField2.GetChecked<FDcExtraTestStructWithColor1>()->ColorField1 == FColor::Blue);
	UTEST_TRUE("Extra FAnyStruct Deserialize", Dest.AnyStructField2.GetChecked<FDcExtraTestStructWithColor1>()->ColorField2 == FColor::Red);

	return true;
}

