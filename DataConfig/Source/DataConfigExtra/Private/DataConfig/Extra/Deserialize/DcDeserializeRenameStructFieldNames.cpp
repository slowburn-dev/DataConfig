#include "DataConfig/Extra/Deserialize/DcDeserializeRenameStructFieldNames.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"

namespace DcExtra
{

struct FRenameStructRootDeserialize : public TSharedFromThis<FRenameStructRootDeserialize>
{
	FDcExtraRenamer Renamer;

	EDcDeserializePredicateResult PredicateIsStructRoot(FDcDeserializeContext& Ctx)
	{
		if (Ctx.TopProperty().IsA<FStructProperty>()
			|| Ctx.TopProperty().IsA<UScriptStruct>())
			return EDcDeserializePredicateResult::Process;
		else
			return EDcDeserializePredicateResult::Pass;
	}

	FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx)
	{
		FDcStructAccess Access;
		DC_TRY(Ctx.Reader->ReadStructRootAccess(Access));
		DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));

		while (true)
		{
			EDcDataEntry Next;
			DC_TRY(Ctx.Reader->PeekRead(&Next));

			if (Next == EDcDataEntry::StructEnd)
			{
				DC_TRY(Ctx.Reader->ReadStructEndAccess(Access));
				DC_TRY(Ctx.Writer->WriteStructEndAccess(Access));
				break;
			}

			FName FieldName;
			DC_TRY(Ctx.Reader->ReadName(&FieldName));
			FName Renamed = Renamer.Execute(FieldName);
			DC_TRY(Ctx.Writer->WriteName(Renamed));

			DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
		}

		return DcOk();
	}

	FDcDeserializeDelegate MakeDeserializeDelegate()
	{
		return FDcDeserializeDelegate::CreateSP(this, &FRenameStructRootDeserialize::HandlerStructRootDeserialize);
	}

	FDcDeserializePredicate MakeDeserializePredicate()
	{
		return FDcDeserializePredicate::CreateSP(this, &FRenameStructRootDeserialize::PredicateIsStructRoot);
	}
};


FDcResult DeserializeStructRenaming(FDcPropertyDatum From, FDcPropertyDatum To, FDcExtraRenamer Renamer)
{
	FDcDeserializer Deserializer;

	DcSetupPropertyPipeDeserializeHandlers(Deserializer);

	TSharedRef<FRenameStructRootDeserialize> RenameStruct = MakeShared<FRenameStructRootDeserialize>();
	RenameStruct->Renamer = Renamer;
	Deserializer.AddPredicatedHandler(
		RenameStruct->MakeDeserializePredicate(),
		RenameStruct->MakeDeserializeDelegate()
	);

	FDcPropertyReader Reader(From);
	FDcPropertyWriter Writer(To);

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	DC_TRY(Ctx.Prepare());

	return Deserializer.Deserialize(Ctx);
}


} // namespace DcExtra


DC_TEST("DataConfig.Extra.Deserialize.StructFieldRename")
{
	using namespace DcExtra;

	FDcTestExtraRenameFrom2 From;
	FDcPropertyDatum FromDatum(&From);

	{
		FDcJsonReader Reader;
		FString Str = TEXT(R"(

			{
				"FromName1" : "Foo",
				"FromStructSet1" : 
				[
					{
						"FromStr1" : "One",
						"FromInt1" : 1,
					},
					{
						"FromStr1" : "Two",
						"FromInt1" : 2,
					}
				]
			}

		)");
		UTEST_OK("Extra Struct Field Rename", Reader.SetNewString(*Str));
		UTEST_OK("Extra Struct Field Rename", DcAutomationUtils::DeserializeFrom(&Reader, FromDatum));
	}

	FDcTestExtraRenameTo2 To;
	FDcPropertyDatum ToDatum(&To);

	UTEST_OK("Extra Struct Field Rename", DcExtra::DeserializeStructRenaming(FromDatum, ToDatum, FDcExtraRenamer::CreateLambda([](const FName& FromName){
		FString FromStr = FromName.ToString();
		if (FromStr.StartsWith(TEXT("From")))
			return FName(TEXT("To") + FromStr.Mid(4));
		else
			return FromName;
	})));


	FDcTestExtraRenameTo2 Expect;
	FDcPropertyDatum ExpectDatum(&Expect);

	{
		FString Str = TEXT(R"(

			{
				"ToName1" : "Foo",
				"ToStructSet1" : 
				[
					{
						"ToStr1" : "One",
						"ToInt1" : 1,
					},
					{
						"ToStr1" : "Two",
						"ToInt1" : 2,
					}
				]
			}

		)");
		FDcJsonReader Reader(Str);
		UTEST_OK("Extra Struct Field Rename", DcAutomationUtils::DeserializeFrom(&Reader, ExpectDatum));
	}

	UTEST_OK("Extra Struct Field Rename", DcAutomationUtils::TestReadDatumEqual(ToDatum, ExpectDatum));

	return true;
}


