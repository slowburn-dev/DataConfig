#include "DataConfig/Extra/SerDe/DcSerDeNested.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"

#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"

namespace DcExtra
{

FDcResult HandlerVector2DDeserialize(FDcDeserializeContext& Ctx)
{
    EDcDataEntry Next;
    DC_TRY(Ctx.Reader->PeekRead(&Next));
    if (Next == EDcDataEntry::MapRoot)
    {
        //  {"X" : 1, "Y" : 2}
        return DcCommonHandlers::HandlerMapToStructDeserialize(Ctx);
    }
    else if (Next == EDcDataEntry::ArrayRoot)
    {
        //  [1, 2]
        DC_TRY(Ctx.Reader->ReadArrayRoot());

        FDcPropertyDatum Datum;
        DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

        FVector2D* Vec2DPtr = (FVector2D*)Datum.DataPtr;

#if ENGINE_MAJOR_VERSION == 5
        static_assert(sizeof(FVector2D) == 16);
        DC_TRY(Ctx.Reader->ReadDouble(&Vec2DPtr->X));
        DC_TRY(Ctx.Reader->ReadDouble(&Vec2DPtr->Y));
#else
        static_assert(sizeof(FVector2D) == 8);
        DC_TRY(Ctx.Reader->ReadFloat(&Vec2DPtr->X));
        DC_TRY(Ctx.Reader->ReadFloat(&Vec2DPtr->Y));
#endif

        DC_TRY(Ctx.Reader->ReadArrayEnd());
        return DcOk();
    }
    else
    {
        return DC_FAIL(DcDSerDe, DataEntryMismatch2)
            << EDcDataEntry::MapRoot << EDcDataEntry::ArrayRoot << Next;
    }

    return DcOk();
}

FDcResult HandlerVector2DSerialize(FDcSerializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

    FVector2D* Vec2DPtr = (FVector2D*)Datum.DataPtr;
    DC_TRY(Ctx.Writer->WriteArrayRoot());

#if ENGINE_MAJOR_VERSION == 5
    static_assert(sizeof(FVector2D) == 16);
    DC_TRY(Ctx.Writer->WriteDouble(Vec2DPtr->X));
    DC_TRY(Ctx.Writer->WriteDouble(Vec2DPtr->Y));
#else
    static_assert(sizeof(FVector2D) == 8);
    DC_TRY(Ctx.Writer->WriteFloat(Vec2DPtr->X));
    DC_TRY(Ctx.Writer->WriteFloat(Vec2DPtr->Y));
#endif

    DC_TRY(Ctx.Writer->WriteArrayEnd());

    return DcOk();
}

#if WITH_EDITORONLY_DATA

namespace Grid2DDetails 
{

FDcResult _GetShapeFromMetaData(FFieldVariant& TopProperty, int& OutWidth, int& OutHeight)
{
    FStructProperty* StructProperty = DcPropertyUtils::CastFieldVariant<FStructProperty>(TopProperty);
    check(StructProperty);

    if (StructProperty->HasMetaData(TEXT("DcWidth")))
    {
        const FString& WidthStr = StructProperty->GetMetaData(TEXT("DcWidth"));
        OutWidth = FCString::Strtoi(*WidthStr, nullptr, 10);
    }
    else
    {
        return DC_FAIL(DcDExtra, NestedMissingMetaData) << TEXT("DcWidth");
    }

    if (StructProperty->HasMetaData(TEXT("DcHeight")))
    {
        const FString& HeightStr = StructProperty->GetMetaData(TEXT("DcHeight"));
        OutHeight = FCString::Strtoi(*HeightStr, nullptr, 10);
    }
    else
    {
        return DC_FAIL(DcDExtra, NestedMissingMetaData) << TEXT("DcHeight");
    }

    return DcOk();
}

} // namespace Grid2DDetails

FDcResult HandlerGrid2DDeserialize(FDcDeserializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

    FDcGrid2D* Grid2DPtr = (FDcGrid2D*)Datum.DataPtr;

    FStructProperty* StructProperty = DcPropertyUtils::CastFieldVariant<FStructProperty>(Ctx.TopProperty());
    check(StructProperty);

    int Width, Height;
    DC_TRY(Grid2DDetails::_GetShapeFromMetaData(Ctx.TopProperty(), Width, Height));

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    EDcDataEntry Next;
    for (int Ix = 0; Ix < Height; Ix++)
    {
        DC_TRY(Ctx.Reader->PeekRead(&Next));
        if (Next != EDcDataEntry::ArrayRoot)
            return DC_FAIL(DcDExtra, NestedGrid2DHeightMismatch) << Height;

        DC_TRY(Ctx.Reader->ReadArrayRoot());

        for (int Jx = 0; Jx < Width; Jx++)
        {
            DC_TRY(Ctx.Reader->PeekRead(&Next));
            if (Next == EDcDataEntry::ArrayEnd)
                return DC_FAIL(DcDExtra, NestedGrid2DWidthMismatch) << Width;

            int DataIx = Grid2DPtr->Data.AddUninitialized();
            DC_TRY(Ctx.Reader->ReadInt32(&Grid2DPtr->Data[DataIx]));
        }

        DC_TRY(Ctx.Reader->PeekRead(&Next));
        if (Next != EDcDataEntry::ArrayEnd)
            return DC_FAIL(DcDExtra, NestedGrid2DWidthMismatch) << Width;

        DC_TRY(Ctx.Reader->ReadArrayEnd());
    }

    DC_TRY(Ctx.Reader->PeekRead(&Next));
    if (Next != EDcDataEntry::ArrayEnd)
        return DC_FAIL(DcDExtra, NestedGrid2DHeightMismatch) << Height;

    DC_TRY(Ctx.Reader->ReadArrayEnd());
    return DcOk();
}

FDcResult HandlerGrid2DSerialize(FDcSerializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

    FDcGrid2D* Grid2DPtr = (FDcGrid2D*)Datum.DataPtr;

    int Width, Height;
    DC_TRY(Grid2DDetails::_GetShapeFromMetaData(Ctx.TopProperty(), Width, Height));

    int ShapeLen = Width * Height;
    if (ShapeLen != Grid2DPtr->Data.Num())
        return DC_FAIL(DcDExtra, NestedGrid2DLenMismatch) << ShapeLen << Grid2DPtr->Data.Num();

    int GridIx = 0;
    DC_TRY(Ctx.Writer->WriteArrayRoot());
    for (int Ix = 0; Ix < Height; Ix++)
    {
        DC_TRY(Ctx.Writer->WriteArrayRoot());
        for (int Jx = 0; Jx < Width; Jx++)
            DC_TRY(Ctx.Writer->WriteInt32(Grid2DPtr->Data[GridIx++]));

        DC_TRY(Ctx.Writer->WriteArrayEnd());
    }
    DC_TRY(Ctx.Writer->WriteArrayEnd());

    return DcOk();
}
#endif // WITH_EDITORONLY_DATA

} // namespace DcExtra


DC_TEST("DataConfig.Extra.SerDe.NestedVec2")
{
    using namespace DcExtra;
    FDcExtraTestNested_Vec2 Dest;
    FDcPropertyDatum DestDatum(&Dest);

    FString Str = TEXT(R"(
        {
            "Vec2ArrayField1" :
            [
                {"X": 1.0, "Y": 2.0},
                {"X": 2.0, "Y": 3.0},
                {"X": 3.0, "Y": 4.0},
                {"X": 4.0, "Y": 5.0},
            ],
            "Vec2ArrayField2" :
            [
                [1.0, 2.0],
                [2.0, 3.0],
                [3.0, 4.0],
                [4.0, 5.0],
            ],
        }
    )");

    {
        FDcJsonReader Reader(Str);
        UTEST_OK("Extra NestedVec2 SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FVector2D>),
                FDcDeserializeDelegate::CreateStatic(HandlerVector2DDeserialize)
            );
        }));

        UTEST_OK("Extra NestedVec2 SerDe", DcAutomationUtils::TestReadDatumEqual(
            FDcPropertyDatum(DcPropertyUtils::FindEffectivePropertyByName(
                FDcExtraTestNested_Vec2::StaticStruct(), TEXT("Vec2ArrayField1")
                ), &Dest.Vec2ArrayField1),
            FDcPropertyDatum(DcPropertyUtils::FindEffectivePropertyByName(
                FDcExtraTestNested_Vec2::StaticStruct(), TEXT("Vec2ArrayField2")
                ), &Dest.Vec2ArrayField2)
        ));
    }


    FString ExpectStr = TEXT(R"(

        {
            "Vec2ArrayField1" : [
                [
                    1,
                    2
                ],
                [
                    2,
                    3
                ],
                [
                    3,
                    4
                ],
                [
                    4,
                    5
                ]
            ],
            "Vec2ArrayField2" : [
                [
                    1,
                    2
                ],
                [
                    2,
                    3
                ],
                [
                    3,
                    4
                ],
                [
                    4,
                    5
                ]
            ]
        }

    )");

    {
        FDcJsonWriter Writer;
        UTEST_OK("Extra NestedVec2 SerDe", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum{&Dest},
        [](FDcSerializeContext& Ctx) {
            Ctx.Serializer->AddPredicatedHandler(
                FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FVector2D>),
                FDcSerializeDelegate::CreateStatic(HandlerVector2DSerialize)
            );
        }));
        Writer.Sb << TCHAR('\n');
        UTEST_EQUAL("Extra NestedVec2 SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(ExpectStr))
    }

    return true;
}

#if WITH_EDITORONLY_DATA


DC_TEST("DataConfig.Extra.SerDe.NestedGrid2D")
{
    using namespace DcExtra;
    FDcExtraTestNested_Grid Dest;
    FDcPropertyDatum DestDatum(&Dest);

    FString Str = TEXT(R"(
        {
            "GridField1" :
            [
                [1,2],
                [3,4],
            ],
            "GridField2" :
            [
                [ 1, 2, 3],
                [ 4, 5, 6],
                [ 7, 8, 9],
                [10,11,12],
            ],
        }
    )");

    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField1"), TEXT("DcWidth"), TEXT("2"));
    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField1"), TEXT("DcHeight"), TEXT("2"));

    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField2"), TEXT("DcWidth"), TEXT("3"));
    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField2"), TEXT("DcHeight"), TEXT("4"));

    {
        FDcJsonReader Reader(Str);
        UTEST_OK("Extra Grid2D SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcDeserializeDelegate::CreateStatic(HandlerGrid2DDeserialize)
            );
        }));

        auto _PopulateRange = [](TArray<int>& Arr, int From, int To) {
            for (int Ix = From; Ix <= To; Ix++)
                Arr.Add(Ix);
        };

        TArray<int> Grid1Data; _PopulateRange(Grid1Data, 1, 4);
        UTEST_TRUE("Extra Grid2D SerDe", Grid1Data == Dest.GridField1.Data);

        TArray<int> Grid2Data; _PopulateRange(Grid2Data, 1, 12);
        UTEST_TRUE("Extra Grid2D SerDe", Grid2Data == Dest.GridField2.Data);
    }

    FString ExpectStr = TEXT(R"(

        {
            "GridField1" : [
                [
                    1,
                    2
                ],
                [
                    3,
                    4
                ]
            ],
            "GridField2" : [
                [
                    1,
                    2,
                    3
                ],
                [
                    4,
                    5,
                    6
                ],
                [
                    7,
                    8,
                    9
                ],
                [
                    10,
                    11,
                    12
                ]
            ]
        }

    )");

    {
        FDcJsonWriter Writer;
        UTEST_OK("Extra Grid2D SerDe", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum{&Dest},
        [](FDcSerializeContext& Ctx) {
            Ctx.Serializer->AddPredicatedHandler(
                FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcSerializeDelegate::CreateStatic(HandlerGrid2DSerialize)
            );
        }));
        Writer.Sb << TCHAR('\n');
        UTEST_EQUAL("Extra Grid2D SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(ExpectStr))
    }

    return true;
}


DC_TEST("DataConfig.Extra.SerDe.NestedGrid2D.Diags")
{
    using namespace DcExtra;

    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField1"), TEXT("DcWidth"), TEXT("2"));
    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField1"), TEXT("DcHeight"), TEXT("2"));

    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField2"), TEXT("DcWidth"), TEXT("3"));
    DcAutomationUtils::AmendMetaData(FDcExtraTestNested_Grid::StaticStruct(), TEXT("GridField2"), TEXT("DcHeight"), TEXT("4"));

    {
        FDcExtraTestNested_Grid Dest;
        FDcPropertyDatum DestDatum(&Dest);

        FString Str = TEXT(R"(
            {
                "GridField1" :
                [
                    [1,2,3],
                    [4,5,6],
                ],
            }
        )");

        FDcJsonReader Reader(Str);
        UTEST_DIAG("Extra Grid2D SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcDeserializeDelegate::CreateStatic(HandlerGrid2DDeserialize)
            );
        }), DcDExtra, NestedGrid2DWidthMismatch);
    }

    {
        FDcExtraTestNested_Grid Dest;
        FDcPropertyDatum DestDatum(&Dest);

        FString Str = TEXT(R"(
            {
                "GridField1" :
                [
                    [1],
                ],
            }
        )");

        FDcJsonReader Reader(Str);
        UTEST_DIAG("Extra Grid2D SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcDeserializeDelegate::CreateStatic(HandlerGrid2DDeserialize)
            );
        }), DcDExtra, NestedGrid2DWidthMismatch);
    }

    {
        FDcExtraTestNested_Grid Dest;
        FDcPropertyDatum DestDatum(&Dest);

        FString Str = TEXT(R"(
            {
                "GridField1" :
                [
                    [1,2],
                ],
            }
        )");

        FDcJsonReader Reader(Str);
        UTEST_DIAG("Extra Grid2D SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcDeserializeDelegate::CreateStatic(HandlerGrid2DDeserialize)
            );
        }), DcDExtra, NestedGrid2DHeightMismatch);
    }

    {
        FDcExtraTestNested_Grid Dest;
        FDcPropertyDatum DestDatum(&Dest);

        FString Str = TEXT(R"(
            {
                "GridField1" :
                [
                    [1,2],
                    [1,2],
                    [1,2],
                ],
            }
        )");

        FDcJsonReader Reader(Str);
        UTEST_DIAG("Extra Grid2D SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {
            Ctx.Deserializer->AddPredicatedHandler(
                FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcDeserializeDelegate::CreateStatic(HandlerGrid2DDeserialize)
            );
        }), DcDExtra, NestedGrid2DHeightMismatch);
    }


    {
        FDcExtraTestNested_Grid Dest;
        Dest.GridField1.Data = {1,2,3};
        FDcJsonWriter Writer;
        UTEST_DIAG("Extra Grid2D SerDe", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum{&Dest},
        [](FDcSerializeContext& Ctx) {
            Ctx.Serializer->AddPredicatedHandler(
                FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcSerializeDelegate::CreateStatic(HandlerGrid2DSerialize)
            );
        }), DcDExtra, NestedGrid2DLenMismatch);
    }

    {
        FDcExtraTestNested_Grid Dest;
        Dest.GridField1.Data = {1,2,3,4,5,6,7};
        FDcJsonWriter Writer;
        UTEST_DIAG("Extra Grid2D SerDe", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum{&Dest},
        [](FDcSerializeContext& Ctx) {
            Ctx.Serializer->AddPredicatedHandler(
                FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsUStruct<FDcGrid2D>),
                FDcSerializeDelegate::CreateStatic(HandlerGrid2DSerialize)
            );
        }), DcDExtra, NestedGrid2DLenMismatch);
    }

    return true;
}
#endif // WITH_EDITORONLY_DATA
