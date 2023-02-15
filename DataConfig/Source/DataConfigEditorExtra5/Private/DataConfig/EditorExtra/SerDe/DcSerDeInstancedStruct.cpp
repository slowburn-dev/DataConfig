#include "DataConfig/EditorExtra/SerDe/DcSerDeInstancedStruct.h"

#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"
#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Writer/DcPutbackWriter.h"

namespace DcExtra
{

FDcResult DcHandlerDeserializeInstancedStruct(
    FDcDeserializeContext& Ctx, 
    TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct
)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FInstancedStruct* InstancedStructPtr = (FInstancedStruct*)Datum.DataPtr;

    if (Next == EDcDataEntry::Nil)
    {
		DC_TRY(Ctx.Reader->ReadNil());
        InstancedStructPtr->Reset();

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

		FField* StructField = Datum.Property.ToField();
		check(StructField);

		if (StructField->HasMetaData(TEXT("BaseStruct")))
		{
			const FString& BaseStructStr = StructField->GetMetaData(TEXT("BaseStruct"));
			UScriptStruct* BaseStruct;
			DC_TRY(DcSerDeUtils::TryLocateObject<UScriptStruct>(BaseStructStr, BaseStruct));

			if (!LoadStruct->IsChildOf(BaseStruct))
				return DC_FAIL(DcDSerDe, StructLhsIsNotChildOfRhs) << LoadStruct->GetFName() << BaseStructStr;
		}

        InstancedStructPtr->InitializeAs(LoadStruct, nullptr);

		DC_TRY(Ctx.Writer->PushTopStructPropertyState({LoadStruct, (void*)InstancedStructPtr->GetMutableMemory()}, Ctx.TopProperty().GetFName()));

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

FDcResult DcHandlerSerializeInstancedStruct(
    FDcSerializeContext& Ctx,
    TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType
)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	FInstancedStruct* InstancedStructPtr = (FInstancedStruct*)Datum.DataPtr;

	if (InstancedStructPtr->IsValid())
	{
		UScriptStruct* LoadStruct = (UScriptStruct*)InstancedStructPtr->GetScriptStruct();

		DC_TRY(Ctx.Writer->WriteMapRoot());
		DC_TRY(Ctx.Writer->WriteString(TEXT("$type")));
		DC_TRY(Ctx.Writer->WriteString(FuncWriteStructType(LoadStruct)));

		DC_TRY(Ctx.Reader->PushTopStructPropertyState(
			FDcPropertyDatum {LoadStruct, InstancedStructPtr->GetMutableMemory()},
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

EDcDeserializePredicateResult PredicateIsInstancedStruct(FDcDeserializeContext& Ctx)
{
    return DcDeserializeUtils::PredicateIsUStruct<FInstancedStruct>(Ctx);
}

FDcResult HandlerInstancedStructDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeInstancedStruct(Ctx, [](FDcDeserializeContext& Ctx, const FString& Str, UScriptStruct*& OutStruct)
	{
		DC_TRY(DcSerDeUtils::TryLocateObject(Str, OutStruct));
		check(OutStruct != nullptr);
		return DcOk();
	});
}

EDcSerializePredicateResult PredicateIsInstancedStruct(FDcSerializeContext& Ctx)
{
    return DcSerializeUtils::PredicateIsUStruct<FInstancedStruct>(Ctx);
}

FDcResult HandlerInstancedStructSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerSerializeInstancedStruct(Ctx, [](UScriptStruct* Struct)
	{
		return DcSerDeUtils::FormatObjectName(Struct);
	});
}

} // namespace DcExtra


DC_TEST("DataConfig.EditorExtra.SerDe.InstancedStruct")
{
	using namespace DcExtra;
	FDcEditorExtra5InstancedStruct1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(

		{
			"InstancedStruct1" : {
				"$type" : "DcExtraTestSimpleStruct1",
				"NameField" : "Foo"
			},
			"InstancedStruct2" : {
				"$type" : "DcExtraTestStructWithColor1",
				"ColorField1" : "#0000FFFF",
				"ColorField2" : "#FF0000FF"
			},
			"InstancedStruct3" : null
		}

	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_OK("Extra FInstancedStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsInstancedStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerInstancedStructDeserialize)
			);
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
			);
		}));

		UTEST_TRUE("Extra FInstancedStruct SerDe", Dest.InstancedStruct1.Get<FDcExtraTestSimpleStruct1>().NameField == TEXT("Foo"));
		UTEST_TRUE("Extra FInstancedStruct SerDe", Dest.InstancedStruct1.Get<FDcExtraTestSimpleStruct1>().IntFieldWithDefault == 253);
		UTEST_TRUE("Extra FInstancedStruct SerDe", Dest.InstancedStruct2.Get<FDcExtraTestStructWithColor1>().ColorField1 == FColor::Blue);
		UTEST_TRUE("Extra FInstancedStruct SerDe", Dest.InstancedStruct2.Get<FDcExtraTestStructWithColor1>().ColorField2 == FColor::Red);
		UTEST_TRUE("Extra FInstancedStruct SerDe", !Dest.InstancedStruct3.IsValid());
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Extra FInstancedStruct SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsInstancedStruct),
				FDcSerializeDelegate::CreateStatic(HandlerInstancedStructSerialize)
			);
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcSerializeDelegate::CreateStatic(HandlerColorSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Extra FInstancedStruct SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}

DC_TEST("DataConfig.EditorExtra.SerDe.InstancedStructBaseStruct")
{
	using namespace DcExtra;
	FDcEditorExtra5InstancedStruct2 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(

		{
			"ShapeField1" : {
				"$type" : "DcStructShapeRectangle",
				"ShapeName" : "MyBox",
				"Height" : 3,
				"Width" : 4
			},
			"ShapeField2" : {
				"$type" : "DcStructShapeCircle",
				"ShapeName" : "MyCircle",
				"Radius" : 5
			}
		}

	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_OK("Extra FInstancedStruct BaseStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsInstancedStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerInstancedStructDeserialize)
			);
		}));

		UTEST_TRUE("Extra FInstancedStruct BaseStruct SerDe", Dest.ShapeField1.Get<FDcStructShapeRectangle>().ShapeName == TEXT("MyBox"));
		UTEST_TRUE("Extra FInstancedStruct BaseStruct SerDe", Dest.ShapeField1.Get<FDcStructShapeRectangle>().Height == 3.0f);
		UTEST_TRUE("Extra FInstancedStruct BaseStruct SerDe", Dest.ShapeField1.Get<FDcStructShapeRectangle>().Width == 4.0f);
		UTEST_TRUE("Extra FInstancedStruct BaseStruct SerDe", Dest.ShapeField2.Get<FDcStructShapeCircle>().ShapeName == TEXT("MyCircle"));
		UTEST_TRUE("Extra FInstancedStruct BaseStruct SerDe", Dest.ShapeField2.Get<FDcStructShapeCircle>().Radius == 5.0f);
	}

	{
		FDcJsonReader Reader(TEXT(R"(

			{
				"ShapeField1" : {
					"$type" : "DcExtraTestSimpleStruct1",
					"NameField" : "Foo"
				}
			}

		)"));
		UTEST_DIAG("Extra FInstancedStruct BaseStruct SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsInstancedStruct),
				FDcDeserializeDelegate::CreateStatic(HandlerInstancedStructDeserialize)
			);
		}), DcDSerDe, StructLhsIsNotChildOfRhs);
	}

	return true;
}
