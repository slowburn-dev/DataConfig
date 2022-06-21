#include "DataConfig/Extra/SerDe/DcSerDeAnyStruct.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"

#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Writer/DcPutbackWriter.h"

namespace DcExtra
{

FDcResult DcHandlerDeserializeAnyStruct(
	FDcDeserializeContext& Ctx,
	TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct
) {
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)Datum.DataPtr;

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		AnyStructPtr->Reset();

		return DcOk();
	}
	else if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		FString Str;
		DC_TRY(Ctx.Reader->ReadString(&Str));
		if (Str != TEXT("$type"))
			return DC_FAIL(DcDSerDe, ExpectMetaType);

		DC_TRY(Ctx.Reader->ReadString(&Str));
		UScriptStruct* LoadStruct = nullptr;
		DC_TRY(FuncLocateStruct(Ctx, Str, LoadStruct));
		check(LoadStruct);

		void* DataPtr = (uint8*)FMemory::Malloc(LoadStruct->GetStructureSize());
		LoadStruct->InitializeStruct(DataPtr);
		FDcAnyStruct TmpAny{DataPtr, LoadStruct};

		*AnyStructPtr = MoveTemp(TmpAny);

		DC_TRY(Ctx.Writer->PushTopStructPropertyState({LoadStruct, (void*)AnyStructPtr->DataPtr}, Ctx.TopProperty().GetFName()));

		FDcPutbackReader PutbackReader(Ctx.Reader);
		PutbackReader.Putback(EDcDataEntry::MapRoot);
		TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::MapRoot << EDcDataEntry::Nil << Next;
	}

}

FDcResult DcHandlerSerializeAnyStruct(FDcSerializeContext& Ctx, TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)Datum.DataPtr;

	if (AnyStructPtr->IsValid())
	{
		DC_TRY(Ctx.Writer->WriteMapRoot());
		DC_TRY(Ctx.Writer->WriteString(TEXT("$type")));
		DC_TRY(Ctx.Writer->WriteString(FuncWriteStructType(AnyStructPtr->StructClass)));

		DC_TRY(Ctx.Reader->PushTopStructPropertyState(
			{AnyStructPtr->StructClass, AnyStructPtr->DataPtr},
			Ctx.TopProperty().GetFName())
		);

		FDcPutbackWriter PutbackWriter{Ctx.Writer};
		PutbackWriter.Putback(EDcDataEntry::MapRoot);
		TDcStoreThenReset<FDcWriter*> RestoreWriter(Ctx.Writer, &PutbackWriter);

		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
	}
	else
	{
		DC_TRY(Ctx.Writer->WriteNil());
	}

	return DcOk();
}


EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx)
{
	return DcDeserializeUtils::PredicateIsUStruct<FDcAnyStruct>(Ctx);
}

FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeAnyStruct(Ctx, [](FDcDeserializeContext& Ctx, const FString& Str, UScriptStruct*& OutStruct)
	{
		DC_TRY(DcSerDeUtils::TryLocateObject(Str, OutStruct));
		check(OutStruct != nullptr);
		return DcOk();
	});
}

EDcSerializePredicateResult PredicateIsDcAnyStruct(FDcSerializeContext& Ctx)
{
	return DcSerializeUtils::PredicateIsUStruct<FDcAnyStruct>(Ctx);
}

FDcResult HandlerDcAnyStructSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerSerializeAnyStruct(Ctx, [](UScriptStruct* Struct)
	{
		return DcSerDeUtils::FormatObjectName(Struct);
	});
}

} // namespace DcExtra

static FDcAnyStruct _IdentityByValue(FDcAnyStruct Handle)
{
	return Handle;
}

DC_TEST("DataConfig.Extra.AnyStructUsage")
{
	//	instantiate from stack allocated structs
	FDcAnyStruct Any1 = new FDcExtraTestSimpleStruct1();
	Any1.GetChecked<FDcExtraTestSimpleStruct1>()->NameField = TEXT("Foo");

	//	supports moving
	FDcAnyStruct Any2 = MoveTemp(Any1);
	check(!Any1.IsValid());
	check(Any2.GetChecked<FDcExtraTestSimpleStruct1>()->NameField == TEXT("Foo"));
	Any2.Reset();

	//	supports shared referencing
	Any2 = new FDcExtraTestSimpleStruct2();
	Any2.GetChecked<FDcExtraTestSimpleStruct2>()->StrField = TEXT("Bar");

	Any1 = Any2;

	check(Any1.DataPtr == Any2.DataPtr);
	check(Any1.StructClass == Any2.StructClass);

	return true;
}

DC_TEST("DataConfig.Extra.AnyStructRefCounts")
{
	{
		FDcAnyStruct Alhpa(new FDcExtraTestSimpleStruct1());
		FDcAnyStruct Beta = Alhpa;

		((void)Beta);

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

DC_TEST("DataConfig.Extra.SerDe.AnyStruct")
{
#if !WITH_METADATA
	DcAutomationUtils::AmendMetaData(FDcExtraTestSimpleStruct1::StaticStruct(), TEXT("IntFieldWithDefault"), TEXT("DcSkip"), TEXT(""));
#endif

	using namespace DcExtra;
	FDcExtraTestWithAnyStruct1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

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
			},
			"AnyStructField3" : null
		}

	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_OK("Extra FAnyStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsDcAnyStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerDcAnyStructDeserialize)
			);
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
			);
		}));

		UTEST_TRUE("Extra FAnyStruct SerDe", Dest.AnyStructField1.GetChecked<FDcExtraTestSimpleStruct1>()->NameField == TEXT("Foo"));
		UTEST_TRUE("Extra FAnyStruct SerDe", Dest.AnyStructField1.GetChecked<FDcExtraTestSimpleStruct1>()->IntFieldWithDefault == 253);
		UTEST_TRUE("Extra FAnyStruct SerDe", Dest.AnyStructField2.GetChecked<FDcExtraTestStructWithColor1>()->ColorField1 == FColor::Blue);
		UTEST_TRUE("Extra FAnyStruct SerDe", Dest.AnyStructField2.GetChecked<FDcExtraTestStructWithColor1>()->ColorField2 == FColor::Red);
		UTEST_TRUE("Extra FAnyStruct SerDe", !Dest.AnyStructField3.IsValid());
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Extra FAnyStruct SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsDcAnyStruct),
				FDcSerializeDelegate::CreateStatic(HandlerDcAnyStructSerialize)
			);
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcSerializeDelegate::CreateStatic(HandlerColorSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Extra FAnyStruct SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}

