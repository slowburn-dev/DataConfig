#include "DataConfig/Extra/SerDe/DcSerDeRoot.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"

#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"

namespace DcExtra
{

FDcResult HandlerRootObjectDeserialize(FDcDeserializeContext& Ctx)
{
    FDcJsonReader* JsonReader = Ctx.Reader->CastByIdChecked<FDcJsonReader>();
    JsonReader->PushTopState(FDcJsonReader::EParseState::Object);
    JsonReader->bTopObjectAtValue = false;
    JsonReader->Keys.AddDefaulted();

    FDcStructAccess Access;
    DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));

    EDcDataEntry CurPeek;
    while (true)
    {
        DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
        if (CurPeek == EDcDataEntry::Ended)
            break;

        FName FieldName;
        DC_TRY(Ctx.Reader->ReadName(&FieldName));
        DC_TRY(Ctx.Writer->WriteName(FieldName));

        DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
    }

    DC_TRY(Ctx.Writer->WriteStructEnd());

    JsonReader->PopTopState(FDcJsonReader::EParseState::Object);
    JsonReader->bTopObjectAtValue = false;
    JsonReader->Keys.Pop();

    return DcOk();
}

FDcResult HandlerRootObjectSerialize(FDcSerializeContext& Ctx)
{
    FDcJsonWriter* JsonWriter = Ctx.Writer->CastByIdChecked<FDcJsonWriter>();
    JsonWriter->States.Push(FDcJsonWriter::EWriteState::Object);
    JsonWriter->State.bTopContainerNotEmpty = false;
    JsonWriter->State.bNeedNewlineAndIndent = false;
    JsonWriter->State.bTopObjectAtValue = false;

    DC_TRY(Ctx.Reader->ReadStructRoot());
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
            DC_TRY(Ctx.Writer->WriteName(Value));
        }
        else
        {
            return DC_FAIL(DcDSerDe, DataEntryMismatch)
                << EDcDataEntry::Name << CurPeek;
        }

        DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
    }
    DC_TRY(Ctx.Reader->ReadStructEnd());

    FDcJsonWriter::EWriteState Popped = JsonWriter->States.Pop();
    if (Popped != FDcJsonWriter::EWriteState::Object
        || JsonWriter->State.bTopObjectAtValue)
        return DC_FAIL(DcDJSON, UnexpectedObjectEnd);

    return DcOk();
}

FDcResult HandlerRootArrayDeserialize(FDcDeserializeContext& Ctx)
{
    FDcJsonReader* JsonReader = Ctx.Reader->CastByIdChecked<FDcJsonReader>();
    JsonReader->PushTopState(FDcJsonReader::EParseState::Array);

    DC_TRY(Ctx.Writer->WriteArrayRoot());

    EDcDataEntry CurPeek;
    while (true)
    {
        DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
        if (CurPeek == EDcDataEntry::Ended)
            break;

        DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
    }

    DC_TRY(Ctx.Writer->WriteArrayEnd());

    JsonReader->PopTopState(FDcJsonReader::EParseState::Array);
    return DcOk();
}

FDcResult HandlerRootArraySerialize(FDcSerializeContext& Ctx)
{
    FDcJsonWriter* JsonWriter = Ctx.Writer->CastByIdChecked<FDcJsonWriter>();
    JsonWriter->States.Push(FDcJsonWriter::EWriteState::Array);
    JsonWriter->State.bTopContainerNotEmpty = false;
    JsonWriter->State.bNeedNewlineAndIndent = false;

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

    FDcJsonWriter::EWriteState Popped = JsonWriter->States.Pop();
    if (Popped != FDcJsonWriter::EWriteState::Array)
        return DC_FAIL(DcDJSON, UnexpectedObjectEnd);

    return DcOk();
}

} // namespace DcExtra


DC_TEST("DataConfig.Extra.SerDe.RootObject")
{
    using namespace DcExtra;
    FDcExtraSimpleStruct Dest;
    FDcPropertyDatum DestDatum(&Dest);

    FString Str = TEXT(R"(

        "Name" : "Foo",
        "Id" : 253,
        "Type" : "Beta"

    )");

    FDcExtraSimpleStruct Expect;
    Expect.Name = TEXT("Foo");
    Expect.Id = 253;
    Expect.Type = EDcExtraTestEnum1::Beta;

    FDcPropertyDatum ExpectDatum(&Expect);

    {
        FDcJsonReader Reader(Str);

        UTEST_OK("Extra RootObject SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsRootProperty),
                FDcDeserializeDelegate::CreateStatic(HandlerRootObjectDeserialize));
        }));
        //  expect FinishRead to fail since this isn't valid JSON, but the reading would
        //  still work for us in this case
        UTEST_DIAG("Extra RootObject SerDe", Reader.FinishRead(), DcDJSON, ExpectStateInProgress);
        UTEST_OK("Extra RootObject SerDe", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
    }

    {
        FDcJsonWriter Writer;
        UTEST_OK("Extra RootObject SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
        [](FDcSerializeContext& Ctx) {
            Ctx.Serializer->AddPredicatedHandler(
                FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsRootProperty),
                FDcSerializeDelegate::CreateStatic(HandlerRootObjectSerialize)
            );
        }));
        Writer.Sb << TCHAR('\n');
        UTEST_EQUAL("Extra RootObject SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
    }

    return true;
}

DC_TEST("DataConfig.Extra.SerDe.RootArray")
{
    using namespace DcExtra;
    using namespace DcPropertyUtils;

    auto ArrProp = FDcPropertyBuilder::Array(
        FDcPropertyBuilder::Enum(
            StaticEnum<EDcExtraTestEnum1>(),
            FDcPropertyBuilder::Int()
        )).LinkOnScope();

    TArray<EDcExtraTestEnum1> Dest;
    FDcPropertyDatum DestDatum(ArrProp.Get(), &Dest);

    FString Str = TEXT(R"(

        "Alpha",
        "Beta",
        "Gamma"

    )");

    TArray<EDcExtraTestEnum1> Expect = {
        EDcExtraTestEnum1::Alpha,
        EDcExtraTestEnum1::Beta,
        EDcExtraTestEnum1::Gamma};
    FDcPropertyDatum ExpectDatum(ArrProp.Get(), &Expect);

    {
        FDcJsonReader Reader(Str);

        UTEST_OK("Extra RootArray SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsRootProperty),
                FDcDeserializeDelegate::CreateStatic(HandlerRootArrayDeserialize));
        }));
        //  expect FinishRead to fail since this isn't valid JSON, but the reading would
        //  still work for us in this case
        UTEST_DIAG("Extra RootArray SerDe", Reader.FinishRead(), DcDJSON, ExpectStateInProgress);
        UTEST_OK("Extra RootArray SerDe", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
    }

    {
        FDcJsonWriter Writer;
        UTEST_OK("Extra RootArray SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
        [](FDcSerializeContext& Ctx) {
            Ctx.Serializer->AddPredicatedHandler(
                FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsRootProperty),
                FDcSerializeDelegate::CreateStatic(HandlerRootArraySerialize)
            );
        }));
        Writer.Sb << TCHAR('\n');
        UTEST_EQUAL("Extra RootArray SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
    }

    return true;
}
