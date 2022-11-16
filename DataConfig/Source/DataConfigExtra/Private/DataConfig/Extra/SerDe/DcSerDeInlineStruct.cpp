#include "DataConfig/Extra/SerDe/DcSerDeInlineStruct.h"
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
#include "DataConfig/Extra/Types/DcInlineStruct.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"

#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Writer/DcPutbackWriter.h"

namespace DcExtra
{

template <typename TInlineStruct>
FDcResult TDcInlineStructDeserialize<TInlineStruct>::DcHandlerDeserializeInlineStruct(FDcDeserializeContext& Ctx,
	TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	TInlineStruct* InlineStructPtr = (TInlineStruct*)Datum.DataPtr;

	if (Next == EDcDataEntry::MapRoot)
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

		if (LoadStruct->GetStructureSize() > TInlineStruct::_CLASS_OFFSET)
		{
			return DC_FAIL(DcDExtra, InlineStructTooBig)
				<< TInlineStruct::_CLASS_OFFSET << LoadStruct->GetFName() << LoadStruct->GetStructureSize();
		}

		LoadStruct->InitializeStruct(&InlineStructPtr->Buffer);
		*InlineStructPtr->GetStructPtr() = LoadStruct;

		DC_TRY(Ctx.Writer->PushTopStructPropertyState(
			{LoadStruct, &InlineStructPtr->Buffer},
			Ctx.TopProperty().GetFName())
			);

		FDcPutbackReader PutbackReader(Ctx.Reader);
		PutbackReader.Putback(EDcDataEntry::MapRoot);
		TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::MapRoot << Next;
	}
}

template <typename TInlineStruct>
EDcDeserializePredicateResult TDcInlineStructDeserialize<TInlineStruct>::PredicateIsDcInlineStruct(
	FDcDeserializeContext& Ctx)
{
	return DcDeserializeUtils::PredicateIsUStruct<TInlineStruct>(Ctx);
}

template <typename TInlineStruct>
FDcResult TDcInlineStructDeserialize<TInlineStruct>::HandlerDcInlineStructDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeInlineStruct(Ctx, [](FDcDeserializeContext& Ctx, const FString& Str, UScriptStruct*& OutStruct)
	{
		DC_TRY(DcSerDeUtils::TryLocateObject(Str, OutStruct));
		check(OutStruct != nullptr);
		return DcOk();
	});

}

template <typename TInlineStruct>
FDcResult TDcInlineStructSerialize<TInlineStruct>::DcHandlerSerializeInlineStruct(FDcSerializeContext& Ctx,
	TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType)
{

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	TInlineStruct* InlineStructPtr = (TInlineStruct*)Datum.DataPtr;

	if (InlineStructPtr->IsValid())
	{
		DC_TRY(Ctx.Writer->WriteMapRoot());
		DC_TRY(Ctx.Writer->WriteString(TEXT("$type")));
		UScriptStruct* Struct = *InlineStructPtr->GetStructPtr();
		DC_TRY(Ctx.Writer->WriteString(FuncWriteStructType(Struct)));

		DC_TRY(Ctx.Reader->PushTopStructPropertyState(
			{Struct, &InlineStructPtr->Buffer},
			Ctx.TopProperty().GetFName()));

		FDcPutbackWriter PutbackWriter{Ctx.Writer};
		PutbackWriter.Putback(EDcDataEntry::MapRoot);
		TDcStoreThenReset<FDcWriter*> RestoreWriter(Ctx.Writer, &PutbackWriter);

		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
	}
	else
	{
		return DC_FAIL(DcDExtra, InlineStructNotSet);
	}

	return DcOk();
}

template <typename TInlineStruct>
EDcSerializePredicateResult TDcInlineStructSerialize<TInlineStruct>::PredicateIsDcInlineStruct(FDcSerializeContext& Ctx)
{
	return DcSerializeUtils::PredicateIsUStruct<TInlineStruct>(Ctx);
}

template <typename TInlineStruct>
FDcResult TDcInlineStructSerialize<TInlineStruct>::HandlerDcInlineStructSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerSerializeInlineStruct(Ctx, [](UScriptStruct* Struct)
	{
		return DcSerDeUtils::FormatObjectName(Struct);
	});
}


} // namespace DcExtra

DC_TEST("DataConfig.Extra.InlineStructUsage")
{
	//	stack allocated usage
	FDcInlineStruct64 Inline1;
	Inline1.Emplace<FColor>(255, 0, 0, 255);
	UTEST_TRUE("Inline Struct", *Inline1.GetChecked<FColor>() == FColor::Red);

	//	support copy 
	FDcInlineStruct64 Inline2 = Inline1;
	UTEST_TRUE("Inline Struct", *Inline2.GetChecked<FColor>() == FColor::Red);

	//	copy constructor properly called
	{
		uint32 CopyCalledCount = 0;
		FDcInlineStruct64 Inline3;
		FDcExtraTestCopyDelegateContainer* Container = Inline3.Emplace<FDcExtraTestCopyDelegateContainer>();
		Container->PropValue = TEXT("Foo");
		Container->NonPropValue = TEXT("Bar");
		Container->CopyAction.BindLambda([&CopyCalledCount]{
			CopyCalledCount++;
		});

		FDcInlineStruct64 Inline4 = Inline3;
		UTEST_EQUAL("Inline Struct", CopyCalledCount, 1);
		UTEST_EQUAL("Inline Struct", Inline4.GetChecked<FDcExtraTestCopyDelegateContainer>()->PropValue, TEXT("Foo"));
		UTEST_EQUAL("Inline Struct", Inline4.GetChecked<FDcExtraTestCopyDelegateContainer>()->NonPropValue, TEXT("Bar"));
	}

	//	destructor properly called
	{
		uint32 DestructCalledCount = 0;
		{
			FDcInlineStruct64 Inline5;
			Inline5.Emplace<FDcExtraTestDestructDelegateContainer>()->DestructAction.BindLambda([&DestructCalledCount]{
				DestructCalledCount++;
			});

			FDcInlineStruct64 Inline6 = Inline5;
		}
		UTEST_EQUAL("Inline Struct", DestructCalledCount, 2);
	}

	return true;
}

DC_TEST("DataConfig.Extra.SerDe.InlineStructOverflow")
{

	using namespace DcExtra;
	FDcExtraTestWithInlineStruct1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(

		{
			"InlineField1" : {
				"$type" : "DcExtraTestStruct128",
			},
		}

	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_DIAG("Extra InlineStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			using Inline64 = TDcInlineStructDeserialize<FDcInlineStruct64>;
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(Inline64::PredicateIsDcInlineStruct),
				FDcDeserializeDelegate::CreateStatic(Inline64::HandlerDcInlineStructDeserialize)
			);
		}), DcDExtra, InlineStructTooBig);
	}

	FDcExtraTestWithInlineStruct1 Src;

	{
		FDcJsonWriter Writer;
		UTEST_DIAG("Extra InlineStruct SerDe", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Src),
		[](FDcSerializeContext& Ctx) {
			using Inline64 = TDcInlineStructSerialize<FDcInlineStruct64>;
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(Inline64::PredicateIsDcInlineStruct),
				FDcSerializeDelegate::CreateStatic(Inline64::HandlerDcInlineStructSerialize)
			);
		}), DcDExtra, InlineStructNotSet);
	}

	return true;
}

DC_TEST("DataConfig.Extra.SerDe.InlineStruct")
{
#if !WITH_METADATA
	DcAutomationUtils::AmendMetaData(FDcExtraTestSimpleStruct1::StaticStruct(), TEXT("IntFieldWithDefault"), TEXT("DcSkip"), TEXT(""));
#endif

	using namespace DcExtra;
	FDcExtraTestWithInlineStruct1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(

		{
			"InlineField1" : {
				"$type" : "DcExtraTestSimpleStruct1",
				"NameField" : "Foo"
			},
			"InlineField2" : {
				"$type" : "DcExtraTestStructWithColor1",
				"ColorField1" : "#0000FFFF",
				"ColorField2" : "#FF0000FF"
			}
		}

	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_OK("Extra InlineStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			using Inline64 = TDcInlineStructDeserialize<FDcInlineStruct64>;
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(Inline64::PredicateIsDcInlineStruct),
				FDcDeserializeDelegate::CreateStatic(Inline64::HandlerDcInlineStructDeserialize)
			);
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
			);
		}));

		FDcExtraTestSimpleStruct1* Struct1Ptr = Dest.InlineField1.GetChecked<FDcExtraTestSimpleStruct1>();
		UTEST_EQUAL("Extra InlineStruct SerDe", Struct1Ptr->NameField, FName(TEXT("Foo")));
		UTEST_EQUAL("Extra InlineStruct SerDe", Struct1Ptr->IntFieldWithDefault, 253);

		FDcExtraTestStructWithColor1* Struct2Ptr = Dest.InlineField2.GetChecked<FDcExtraTestStructWithColor1>();
		UTEST_EQUAL("Extra InlineStruct SerDe", Struct2Ptr->ColorField1, FColor::Blue);
		UTEST_EQUAL("Extra InlineStruct SerDe", Struct2Ptr->ColorField2, FColor::Red);
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Extra InlineStruct SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			using Inline64 = TDcInlineStructSerialize<FDcInlineStruct64>;
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(Inline64::PredicateIsDcInlineStruct),
				FDcSerializeDelegate::CreateStatic(Inline64::HandlerDcInlineStructSerialize)
			);
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcSerializeDelegate::CreateStatic(HandlerColorSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Extra InlineStruct SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}


