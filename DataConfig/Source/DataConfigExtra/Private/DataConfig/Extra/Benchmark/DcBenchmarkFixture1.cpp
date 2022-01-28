#include "DataConfig/Extra/Benchmark/DcBenchmarkFixture1.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Extra/Misc/DcBench.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Deserialize/Handlers/Json/DcJsonCommonDeserializers.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "Misc/FileHelper.h"

namespace DcBenchmarkDetails
{

FDcResult HandlerCanadaCoordsDeserialize(FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	FDcStructAccess Access{FDcStructAccess::EFlag::WriteCheckName, TEXT("DcCanadaCoords") };
	DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));
	DC_TRY(Ctx.Writer->WriteName(TEXT("data")));
	DC_TRY(Ctx.Writer->WriteArrayRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
	}

	DC_TRY(Ctx.Writer->WriteArrayEnd());
	DC_TRY(Ctx.Writer->WriteStructEndAccess(Access));
	DC_TRY(Ctx.Reader->ReadArrayEnd());
	return DcOk();
}

FDcResult HandlerCanadaCoordsSerialize(FDcSerializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadStructRoot());
	DC_TRY(Ctx.Reader->ReadName(nullptr));
	DC_TRY(Ctx.Reader->ReadArrayRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;
		
		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadArrayEnd());
	DC_TRY(Ctx.Reader->ReadStructEnd());
	return DcOk();
}

FDcResult HandlerVector2DDeserialize(FDcDeserializeContext& Ctx)
{
	FVector2D Vec;
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadFloat(&Vec.X));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec.Y));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FVector2D* Vec2DPtr = (FVector2D*)Datum.DataPtr;
	*Vec2DPtr = Vec;

	return DcOk();
}

FDcResult HandlerVector2DSerialize(FDcSerializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	FVector2D* Vec2DPtr = (FVector2D*)Datum.DataPtr;

	DC_TRY(Ctx.Writer->WriteArrayRoot());
	DC_TRY(Ctx.Writer->WriteFloat(Vec2DPtr->X));
	DC_TRY(Ctx.Writer->WriteFloat(Vec2DPtr->Y));
	DC_TRY(Ctx.Writer->WriteArrayEnd());

	return DcOk();
}

} // namespace DcBenchmarkDetails

DC_TEST("DataConfigBenchmark.Canada")
{
	using namespace DcBenchmarkDetails;
	FString JsonStr;
	verify(FFileHelper::LoadFileToString(JsonStr, *DcGetFixturePath(TEXT("LargeFixtures/canada.json"))));
	FDcCanadaRoot Root;
	auto _LoadJson = [](FString JsonStr, FDcPropertyDatum Datum)
	{
		FDcJsonReader Reader(JsonStr);
		return DcAutomationUtils::DeserializeFrom(&Reader, Datum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcCanadaCoords>),
				FDcDeserializeDelegate::CreateStatic(HandlerCanadaCoordsDeserialize)
			);
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FVector2D>),
				FDcDeserializeDelegate::CreateStatic(HandlerVector2DDeserialize)
			);
		});
	};

	UTEST_OK("Canada Benchmark", _LoadJson(JsonStr, FDcPropertyDatum(&Root)));
	FDcMsgPackWriter MsgPackWriter;
	{
		FDcJsonReader JsonReader(JsonStr);
		FDcPipeVisitor PipeVisitor(&JsonReader, &MsgPackWriter);

		//	read json number to msgpack float
		PipeVisitor.PeekVisit.BindLambda([](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl)
		{
			if (Next == EDcDataEntry::Double)
			{
				OutControl = EPipeVisitControl::SkipContinue;
				float Value;
				DC_TRY(Visitor->Reader->ReadFloat(&Value));
				DC_TRY(Visitor->Writer->WriteFloat(Value));
				return DcOk();
			}

			OutControl = EPipeVisitControl::Pass;
			return DcOk();
		});

		UTEST_OK("Canada Benchmark", PipeVisitor.PipeVisit());
	}
	FDcMsgPackWriter::BufferType& Buffer = MsgPackWriter.GetMainBuffer();

	//	Json Deserialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcCanadaRoot Data;
			FDcResult Result = _LoadJson(JsonStr, FDcPropertyDatum(&Data));
			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Canada Json Deserialize"), JsonStr.Len(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	//	Json Serialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcJsonWriter Writer;
			FDcResult Result = DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Root),
			[](FDcSerializeContext& Ctx) {
				Ctx.Serializer->AddPredicatedHandler(
					FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcCanadaCoords>),
					FDcSerializeDelegate::CreateStatic(HandlerCanadaCoordsSerialize)
				);

				Ctx.Serializer->AddPredicatedHandler(
					FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FVector2D>),
					FDcSerializeDelegate::CreateStatic(HandlerVector2DSerialize)
				);
			});
			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Canada Json Serialize"), JsonStr.Len(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	//	MsgPack Deserialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcCanadaRoot Data;
			FDcMsgPackReader Reader(FDcBlobViewData::From(Buffer));
			FDcResult Result = DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Data),
			[](FDcDeserializeContext& Ctx) {
				DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, EDcMsgPackDeserializeType::Default);
				Ctx.Deserializer->AddPredicatedHandler(
					FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcCanadaCoords>),
					FDcDeserializeDelegate::CreateStatic(HandlerCanadaCoordsDeserialize)
				);
				Ctx.Deserializer->AddPredicatedHandler(
					FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FVector2D>),
					FDcDeserializeDelegate::CreateStatic(HandlerVector2DDeserialize)
				);
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing);
			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Canada MsgPack Serialize"), Buffer.Num(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	//	MsgPack Serialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcMsgPackWriter Writer;
			FDcResult Result = DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Root), 
			[](FDcSerializeContext& Ctx) {
				DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, EDcMsgPackSerializeType::Default);

				Ctx.Serializer->AddPredicatedHandler(
					FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcCanadaCoords>),
					FDcSerializeDelegate::CreateStatic(HandlerCanadaCoordsSerialize)
				);

				Ctx.Serializer->AddPredicatedHandler(
					FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FVector2D>),
					FDcSerializeDelegate::CreateStatic(HandlerVector2DSerialize)
				);
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing);
			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Canada MsgPack Deserialize"), Buffer.Num(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	return true;
}

namespace DcBenchmarkDetails
{

FDcResult HandlerIsCorpusRootDeserialize(FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Writer->WriteStructRoot());
	DC_TRY(Ctx.Writer->WriteName(TEXT("data")));
	DC_TRY(Ctx.Writer->WriteArrayRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		//	read until EOF as we're processing ndjson
		if (CurPeek == EDcDataEntry::Ended)
			break;

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
	}

	DC_TRY(Ctx.Writer->WriteArrayEnd());
	DC_TRY(Ctx.Writer->WriteStructEnd());

	return DcOk();
}

FDcResult HandlerIsCorpusRootSerialize(FDcSerializeContext& Ctx)
{
	FDcMsgPackWriter* MsgPackWriter = Ctx.Writer->CastById<FDcMsgPackWriter>();

	DC_TRY(Ctx.Reader->ReadStructRoot());
	DC_TRY(Ctx.Reader->ReadName(nullptr));
	DC_TRY(Ctx.Reader->ReadArrayRoot());

	if (MsgPackWriter)
		DC_TRY(MsgPackWriter->WriteArrayRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;

		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));

		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;

		FDcJsonWriter* JsonWriter = Ctx.Writer->CastById<FDcJsonWriter>();
		if (JsonWriter)
		{
			//	write as ndjson
			JsonWriter->CancelWriteComma();
			JsonWriter->Sb.Append(TEXT('\n'));
		}
	}

	if (MsgPackWriter)
		DC_TRY(MsgPackWriter->WriteArrayEnd());

	DC_TRY(Ctx.Reader->ReadArrayEnd());
	DC_TRY(Ctx.Reader->ReadStructEnd());
	return DcOk();
}

template<typename TCtx>
bool IsCorpusNullableField(TCtx& Ctx)
{
	FString TopName = Ctx.TopProperty().GetName();
	if (TopName == TEXT("structuredName")
		|| TopName == TEXT("year"))
	{
		return true;
	}
	return false;
}

FDcResult HandlerNullableDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Ctx.TopProperty());

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());

		switch (Actual)
		{
			case EDcDataEntry::String:
			{
				DC_TRY(Ctx.Writer->WriteString(TEXT("<empty>")));
				break;
			}
			case EDcDataEntry::Int32:
			{
				DC_TRY(Ctx.Writer->WriteInt32(0));
				break;
			}
			default:
				return DcNoEntry();
		}

		return DcOk();
	}
	else if (Next == EDcDataEntry::String)
	{
		return DcJsonHandlers::HandlerStringDeserialize(Ctx);
	}
	else if (DcTypeUtils::IsNumericDataEntry(Next))
	{
		return DcJsonHandlers::HandlerNumericDeserialize(Ctx);
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch)
			<< Next << Actual;
	}
}

FDcResult HandlerNullableSerialize(FDcSerializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

		if (Value == TEXT("<empty>"))
			DC_TRY(Ctx.Writer->WriteNil());
		else
			DC_TRY(Ctx.Writer->WriteString(Value));

		return DcOk();
	}
	else if (Next == EDcDataEntry::Int32)
	{
		int Value;
		DC_TRY(Ctx.Reader->ReadInt32(&Value));
		if (Value == 0)
			DC_TRY(Ctx.Writer->WriteNil());
		else
			DC_TRY(Ctx.Writer->WriteInt32(Value));

		return DcOk();
	}
	else 
	{
		return DcSerializeUtils::RecursiveSerialize(Ctx);
	}
}

} // namespace DcBenchmarkDetails

DC_TEST("DataConfigBenchmark.Corpus")
{
	using namespace DcBenchmarkDetails;
	FString JsonStr;
	verify(FFileHelper::LoadFileToString(JsonStr, *DcGetFixturePath(TEXT("LargeFixtures/corpus.ndjson"))));
	FDcCorpusRoot Root;
	auto _LoadJson = [](FString JsonStr, FDcPropertyDatum Datum)
	{
		FDcJsonReader Reader(JsonStr);
		return DcAutomationUtils::DeserializeFrom(&Reader, Datum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcCorpusRoot>),
				FDcDeserializeDelegate::CreateStatic(HandlerIsCorpusRootDeserialize)
			);

			Ctx.Deserializer->PredicatedDeserializers.Insert(
				MakeTuple(
					FDcDeserializePredicate::CreateLambda([](FDcDeserializeContext& Ctx)
					{
						return DcBenchmarkDetails::IsCorpusNullableField(Ctx)
							? EDcDeserializePredicateResult::Process
							: EDcDeserializePredicateResult::Pass;
					}),
					FDcDeserializeDelegate::CreateStatic(HandlerNullableDeserialize)
				), 
				0	// insert at 0, before usual numeric handlers
			);
		});
	};

	UTEST_OK("Corpus Benchmark", _LoadJson(JsonStr, FDcPropertyDatum(&Root)));
	FDcMsgPackWriter MsgPackWriter;
	{
		FDcJsonReader JsonReader(JsonStr);
		FDcPipeVisitor PipeVisitor(&JsonReader, &MsgPackWriter);

		//	read json number to msgpack int32
		PipeVisitor.PeekVisit.BindLambda([](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl)
		{
			if (Next == EDcDataEntry::Double)
			{
				OutControl = EPipeVisitControl::SkipContinue;
				int32 Value;
				DC_TRY(Visitor->Reader->ReadInt32(&Value));
				DC_TRY(Visitor->Writer->WriteInt32(Value));
				return DcOk();
			}

			OutControl = EPipeVisitControl::Pass;
			return DcOk();
		});

		UTEST_OK("Corpus Benchmark", PipeVisitor.PipeVisit());
	}
	FDcMsgPackWriter::BufferType& Buffer = MsgPackWriter.GetMainBuffer();

	//	Json Deserialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcCorpusRoot Data;
			FDcResult Result = _LoadJson(JsonStr, FDcPropertyDatum(&Data));
			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Corpus Json Deserialize"), JsonStr.Len(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	//	Json Serialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcCondensedJsonWriter Writer;
			FDcResult Result = DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Root), 
			[](FDcSerializeContext& Ctx) {
				Ctx.Serializer->AddPredicatedHandler(
					FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcCorpusRoot>),
					FDcSerializeDelegate::CreateStatic(HandlerIsCorpusRootSerialize)
				);

				Ctx.Serializer->PredicatedSerializers.Insert(
					MakeTuple(
						FDcSerializePredicate::CreateLambda([](FDcSerializeContext& Ctx)
						{
							return DcBenchmarkDetails::IsCorpusNullableField(Ctx)
								? EDcSerializePredicateResult::Process
								: EDcSerializePredicateResult::Pass;
						}),
						FDcSerializeDelegate::CreateStatic(HandlerNullableSerialize)
					), 
					0	// insert at 0, before usual numeric handlers
				);
			});

			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Corpus Json Serialize"), JsonStr.Len(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	//	MsgPack Deserialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcCorpusRoot Data;
			FDcMsgPackReader Reader(FDcBlobViewData::From(Buffer));
			FDcResult Result = DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Data),
			[](FDcDeserializeContext& Ctx) {
				DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, EDcMsgPackDeserializeType::Default);
				Ctx.Deserializer->AddPredicatedHandler(
					FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcCorpusRoot>),
					FDcDeserializeDelegate::CreateStatic(HandlerIsCorpusRootDeserialize)
				);

				Ctx.Deserializer->PredicatedDeserializers.Insert(
					MakeTuple(
						FDcDeserializePredicate::CreateLambda([](FDcDeserializeContext& Ctx)
						{
							return DcBenchmarkDetails::IsCorpusNullableField(Ctx)
								? EDcDeserializePredicateResult::Process
								: EDcDeserializePredicateResult::Pass;
						}),
						FDcDeserializeDelegate::CreateStatic(HandlerNullableDeserialize)
					), 
					0	// insert at 0, before usual numeric handlers
				);
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing);

			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Corpus MsgPack Deserialize"), Buffer.Num(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	//	MsgPack Serialize
	{
		FDcBenchStat Stat = DcBenchStats([&]
		{
			FDcMsgPackWriter Writer;
			FDcResult Result = DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Root), 
			[](FDcSerializeContext& Ctx) {
				DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, EDcMsgPackSerializeType::Default);

				Ctx.Serializer->AddPredicatedHandler(
					FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcCorpusRoot>),
					FDcSerializeDelegate::CreateStatic(HandlerIsCorpusRootSerialize)
				);

				Ctx.Serializer->PredicatedSerializers.Insert(
					MakeTuple(
						FDcSerializePredicate::CreateLambda([](FDcSerializeContext& Ctx)
						{
							return DcBenchmarkDetails::IsCorpusNullableField(Ctx)
								? EDcSerializePredicateResult::Process
								: EDcSerializePredicateResult::Pass;
						}),
						FDcSerializeDelegate::CreateStatic(HandlerNullableSerialize)
					), 
					0	// insert at 0, before usual numeric handlers
				);
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing);

			return Result.Ok();
		});

		FString Output = DcFormatBenchStats(TEXT("Corpus MsgPack Serialize"), Buffer.Num(), Stat);
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Output);
		if (!Stat.bAllOk)
			return false;
	}

	return true;
}


