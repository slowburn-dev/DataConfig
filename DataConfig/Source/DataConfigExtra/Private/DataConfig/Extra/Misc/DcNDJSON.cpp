#include "DataConfig/Extra/Misc/DcNDJSON.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializerSetup.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"

namespace DcExtra
{

namespace NDJSONDetails
{

static TOptional<FDcDeserializer> Deserializer;
static void LazyInitializeDeserializer()
{
    if (Deserializer.IsSet())
        return;
    
    Deserializer.Emplace();
    DcSetupJsonDeserializeHandlers(Deserializer.GetValue());

    Deserializer->AddPredicatedHandler(
        FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsRootProperty),
        FDcDeserializeDelegate::CreateLambda([](FDcDeserializeContext& Ctx) -> FDcResult
        {
            if (!Ctx.TopProperty().IsA<FArrayProperty>())
                return DC_FAIL(DcDReadWrite, PropertyMismatch)
                    << TEXT("Array") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

            DC_TRY(Ctx.Writer->WriteArrayRoot());
            EDcDataEntry CurPeek;
            while (true)
            {
                DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
                //  read until EOF as we're processing ndjson
                if (CurPeek == EDcDataEntry::Ended)
                    break;

                DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
            }

            DC_TRY(Ctx.Writer->WriteArrayEnd());
            return DcOk();
        })
    );
}


static TOptional<FDcSerializer> Serializer;
static void LazyInitializeSerializer()
{
    if (Serializer.IsSet())
        return;

    Serializer.Emplace();
    DcSetupJsonSerializeHandlers(Serializer.GetValue());

    Serializer->AddPredicatedHandler(
        FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsRootProperty),
        FDcSerializeDelegate::CreateLambda([](FDcSerializeContext& Ctx) -> FDcResult{
            if (!Ctx.TopProperty().IsA<FArrayProperty>())
                return DC_FAIL(DcDReadWrite, PropertyMismatch)
                    << TEXT("Array") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

            FDcJsonWriter* JsonWriter = Ctx.Writer->CastByIdChecked<FDcJsonWriter>();

            DC_TRY(Ctx.Reader->ReadArrayRoot());

            EDcDataEntry CurPeek;
            while (true)
            {
                DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
                if (CurPeek == EDcDataEntry::ArrayEnd)
                    break;

                DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));

                JsonWriter->CancelWriteComma();
                JsonWriter->Sb << TCHAR('\n');
            }

            DC_TRY(Ctx.Reader->ReadArrayEnd());

            return DcOk();
        })
    );
}

} // namespace NDJSONDetails

FDcResult LoadNDJSON(const TCHAR* Str, FDcPropertyDatum Datum)
{
    using namespace NDJSONDetails;

    FDcJsonReader Reader(Str);
    FDcPropertyWriter Writer(Datum);

    LazyInitializeDeserializer();

    FDcDeserializeContext Ctx;
    Ctx.Reader = &Reader;
    Ctx.Writer = &Writer;
    Ctx.Deserializer = &Deserializer.GetValue();
    Ctx.Properties.Add(Datum.Property);
    DC_TRY(Ctx.Prepare());
    DC_TRY(Deserializer->Deserialize(Ctx));

    return DcOk();
}


FDcResult SaveNDJSON(FDcPropertyDatum Datum, FString& OutStr)
{
    using namespace NDJSONDetails;

    FDcJsonWriter::ConfigType Config = {
        TEXT(" "),
        TEXT(" "),
        TEXT(""),
        TEXT(" "),
        false,
        false
    };

    FDcJsonWriter Writer(Config);
    FDcPropertyReader Reader(Datum);

    LazyInitializeSerializer();

    FDcSerializeContext Ctx;
    Ctx.Reader = &Reader;
    Ctx.Writer = &Writer;
    Ctx.Serializer = &Serializer.GetValue();
    DC_TRY(Ctx.Prepare());
    DC_TRY(Serializer->Serialize(Ctx));

    OutStr = Writer.Sb.ToString();
    return DcOk();
}

} // namespace DcExtra

DC_TEST("DataConfig.Extra.SerDe.NDJSON")
{
    using namespace DcExtra;

    TArray<FDcExtraSimpleStruct> Dest;

    FString Str = TEXT(R"(

        { "Name" : "Foo", "Id" : 1, "Type" : "Alpha" }
        { "Name" : "Bar", "Id" : 2, "Type" : "Beta" }
        { "Name" : "Baz", "Id" : 3, "Type" : "Gamma" }

    )");


    UTEST_OK("Extra NDJSON", LoadNDJSON(*Str, Dest));

    FString SavedStr;
    UTEST_OK("Extra NDJSON", SaveNDJSON(Dest, SavedStr));

    UTEST_EQUAL("Extra NDJSON", SavedStr, DcAutomationUtils::DcReindentStringLiteral(Str));
    return true;
};
