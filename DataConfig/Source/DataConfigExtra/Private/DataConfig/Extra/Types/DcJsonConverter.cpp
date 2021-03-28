#include "DataConfig/Extra/Types/DcJsonConverter.h"

#include "DataConfig/DcEnv.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

#include "JsonObjectConverter.h"

namespace DcExtra
{

namespace JsonConverterDetails
{
static TOptional<FDcDeserializer> Deserializer;

static void LazyInitializeDeserializer()
{
	if (Deserializer.IsSet())
		return;
	
	Deserializer.Emplace();
	DcSetupJsonDeserializeHandlers(Deserializer.GetValue());
}

} // namespace JsonConverterDetails


bool JsonObjectReaderToUStruct(FDcReader* Reader, FDcPropertyDatum Datum)
{
	FDcResult Ret = [&]() -> FDcResult {
		using namespace JsonConverterDetails;
		LazyInitializeDeserializer();

		FDcPropertyWriter Writer(Datum);

		FDcDeserializeContext Ctx;
		Ctx.Reader = Reader;
		Ctx.Writer = &Writer;
		Ctx.Deserializer = &Deserializer.GetValue();
		Ctx.Properties.Push(Datum.Property);
		DC_TRY(Ctx.Prepare());

		DC_TRY(Deserializer->Deserialize(Ctx));
		return DcOk();
	}();

	if (!Ret.Ok())
	{
		DcEnv().FlushDiags();
		return false;
	}
	else
	{
		return true;
	}
}

} // namespace DcExtra


DC_TEST("DataConfig.Core.Deserialize.JsonConverter")
{
	FString Str = TEXT(R"(
		{
			"StrField" : "Foo",
			"IntField" : 253,
			"BoolField" : true
		}
	)");
	
	FDcTestJsonConverter1 Lhs;
	bool LhsOk = DcExtra::JsonObjectStringToUStruct(Str, &Lhs);

	FDcTestJsonConverter1 Rhs;
	bool RhsOk = FJsonObjectConverter::JsonObjectStringToUStruct(Str, &Rhs, 0, 0);
	
	UTEST_TRUE("JsonObjectStringToUStruct", LhsOk);
	UTEST_TRUE("JsonObjectStringToUStruct", RhsOk);
	
	UTEST_OK("Deserialize into FDcTestStructRefs1", DcAutomationUtils::TestReadDatumEqual(
		FDcPropertyDatum(FDcTestJsonConverter1::StaticStruct(), &Lhs), 
		FDcPropertyDatum(FDcTestJsonConverter1::StaticStruct(), &Rhs)
	));

	return true;
}
