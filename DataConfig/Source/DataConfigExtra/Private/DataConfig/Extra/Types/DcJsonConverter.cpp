#include "DataConfig/Extra/Types/DcJsonConverter.h"

#include "DataConfig/DcEnv.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializerSetup.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"

#include "JsonObjectConverter.h"

namespace DcExtra
{

namespace JsonConverterDetails
{

static FDcResult HandlerStructRootSerializeCamelCase(FDcSerializeContext& Ctx)
{
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
			DC_TRY(Ctx.Writer->WriteString(FJsonObjectConverter::StandardizeCase(Value.ToString())));
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

static FDcResult HandlerClassRootSerializeCamelCase(FDcSerializeContext& Ctx)
{
	FDcClassAccess Access{FDcClassAccess::EControl::ExpandObject};
	DC_TRY(Ctx.Reader->ReadClassRootAccess(Access));
	DC_TRY(Ctx.Writer->WriteMapRoot());


	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ClassEnd)
		{
			break;
		}
		else if (CurPeek == EDcDataEntry::Name)
		{
			FName Value;
			DC_TRY(Ctx.Reader->ReadName(&Value));
			DC_TRY(Ctx.Writer->WriteString(FJsonObjectConverter::StandardizeCase(Value.ToString())));
		}
		else
		{
			return DC_FAIL(DcDSerDe, DataEntryMismatch)
				<< EDcDataEntry::Name << CurPeek;
		}

		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

static TOptional<FDcDeserializer> Deserializer;
static TOptional<FDcSerializer> Serializer;

static void LazyInitializeSerializer()
{
	if (Serializer.IsSet())
		return;

	Serializer.Emplace();
	DcSetupJsonSerializeHandlers(Serializer.GetValue());

	//	handle camelCase member names
	Serializer->UClassSerializerMap[UScriptStruct::StaticClass()] = FDcSerializeDelegate::CreateStatic(HandlerStructRootSerializeCamelCase);
	Serializer->UClassSerializerMap[UClass::StaticClass()] = FDcSerializeDelegate::CreateStatic(HandlerClassRootSerializeCamelCase);
	Serializer->FieldClassSerializerMap[FStructProperty::StaticClass()] = FDcSerializeDelegate::CreateStatic(HandlerStructRootSerializeCamelCase);
}


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

bool UStructToJsonObjectString(FDcPropertyDatum Datum, FDcWriter* Writer)
{
	FDcResult Ret = [&]() -> FDcResult
	{
		using namespace JsonConverterDetails;
		LazyInitializeSerializer();

		FDcPropertyReader Reader(Datum);

		FDcSerializeContext Ctx;
		Ctx.Reader = &Reader;
		Ctx.Writer = Writer;
		Ctx.Serializer = &Serializer.GetValue();
		DC_TRY(Ctx.Prepare());

		DC_TRY(Serializer->Serialize(Ctx));
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

DC_TEST("DataConfig.Extra.JsonConverterBlurb")
{
	//	https://slowburn.dev/blog/dataconfig-1-2/
	{
		// Deserialize JSON in UE Property System
		FString Str = TEXT(R"(
		{
		  "Name": "Bad Boys",
		  "ReleaseDate": "1995-4-7T00:00:00",
		  "Genres": [
		    "Action",
		    "Comedy"
		  ]
		}
		)");

		FMovie m;
		bool Ok = FJsonObjectConverter::JsonObjectStringToUStruct(Str, &m, 0, 0);

		FString name = m.Name;
		// Bad Boys
	}

	{
		FString Str = TEXT(R"(
		{
		  "Name": "Bad Boys",
		  "ReleaseDate": "1995-4-7T00:00:00",
		  "Genres": [
		    "Action",
		    "Comedy"
		  ]
		}
		)");

		FMovie m;
		bool Ok = DcExtra::JsonObjectStringToUStruct(Str, &m);

		FString name = m.Name;
		// Bad Boys
	}

	return true;
}

DC_TEST("DataConfig.Extra.JsonConverter")
{
	FString Str = TEXT(R"(
		{
			"strField" : "Foo",
			"nestField" : {
				"strArrayField" : [
					"One",
					"Two",
					"Three"
				],
				"strIntMapField" : {
					"One": 1,
					"Two": 2,
					"Three": 3
				}
			},
			"intField" : 253,
			"boolField" : true
		}
	)");

	FDcTestJsonConverter1 Data = {
		"Foo",
		{
			{TEXT("One"), TEXT("Two"), TEXT("Three")},
			{
				{TEXT("One"), 1},
				{TEXT("Two"), 2},
				{TEXT("Three"), 3},
			}
		},
		253,
		true
	};
	
	{
		FDcTestJsonConverter1 Lhs;
		bool LhsOk = DcExtra::JsonObjectStringToUStruct(Str, &Lhs);

		FDcTestJsonConverter1 Rhs;
		bool RhsOk = FJsonObjectConverter::JsonObjectStringToUStruct(Str, &Rhs, 0, 0);
		
		UTEST_TRUE("JsonObjectStringToUStruct", LhsOk);
		UTEST_TRUE("JsonObjectStringToUStruct", RhsOk);
		
		UTEST_OK("JsonConverter", DcAutomationUtils::TestReadDatumEqual(
			FDcPropertyDatum(&Lhs), 
			FDcPropertyDatum(&Rhs)
		));
	}

	{
		FString Lhs;
		bool LhsOk = DcExtra::UStructToJsonObjectString(Data, Lhs);

		FString Rhs;
		bool RhsOk = FJsonObjectConverter::UStructToJsonObjectString(Data, Rhs, 0, 0);

		UTEST_TRUE("UStructToJsonObjectString", LhsOk);
		UTEST_TRUE("UStructToJsonObjectString", RhsOk);

		UTEST_EQUAL("JsonConverter", Lhs, Rhs);
	}

	return true;
}
