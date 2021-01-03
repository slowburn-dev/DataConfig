#include "DataConfig/Extra/Deserialize/DcDeserializeRenaming.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Property/DcPropertyReader.h"

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

	FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
	{
		FDcStructStat StructStat;
		DC_TRY(Ctx.Reader->ReadStructRoot(&StructStat));
		DC_TRY(Ctx.Writer->WriteStructRoot(StructStat));

		while (true)
		{
			EDcDataEntry Next;
			DC_TRY(Ctx.Reader->PeekRead(&Next));

			if (Next == EDcDataEntry::StructEnd)
			{
				DC_TRY(Ctx.Reader->ReadStructEnd(&StructStat));
				DC_TRY(Ctx.Writer->WriteStructEnd(StructStat));
				break;
			}

			FName FieldName;
			DC_TRY(Ctx.Reader->ReadName(&FieldName));
			FName Renamed = Renamer.Execute(FieldName);
			DC_TRY(Ctx.Writer->WriteName(Renamed));

			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
		}

		return DcOkWithProcessed(OutRet);
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

static FDcResult HandlerPipeDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer);
	return DcOkWithProcessed(OutRet);
}


FDcResult DeserializeRenaming(FDcPropertyDatum From, FDcPropertyDatum To, FDcExtraRenamer Renamer)
{
	FDcDeserializer Deserializer;

	TSharedRef<FRenameStructRootDeserialize> RenameStruct = MakeShared<FRenameStructRootDeserialize>();
	RenameStruct->Renamer = Renamer;
	Deserializer.AddPredicatedHandler(
		RenameStruct->MakeDeserializePredicate(),
		RenameStruct->MakeDeserializeDelegate()
	);

	FDcDeserializeDelegate::CreateStatic(HandlerPipeDeserialize);

	DcPropertyUtils::VisitAllEffectivePropertyClass([&](FFieldClass* FieldClass){
		Deserializer.AddDirectHandler(
			FieldClass,
			FDcDeserializeDelegate::CreateStatic(HandlerPipeDeserialize));
	});

	FDcPropertyReader Reader(From);
	FDcPropertyWriter Writer(To);

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(To.Property);
	Ctx.Prepare();

	return Deserializer.Deserialize(Ctx);
}


}	//	namespace DcExtra


DC_TEST("DataConfig.Extra.Deserialize.StructFieldRename")
{
	using namespace DcExtra;

	FDcTestExtraRenameFrom1 From;
	FDcPropertyDatum FromDatum(FDcTestExtraRenameFrom1::StaticStruct(), &From);

	FDcTestExtraRenameTo1 To;
	FDcPropertyDatum ToDatum(FDcTestExtraRenameTo1::StaticStruct(), &To);

	UTEST_OK("Extra Struct Field Rename", DcExtra::DeserializeRenaming(FromDatum, ToDatum, FDcExtraRenamer::CreateLambda([](const FName& FromName){
		FString FromStr = FromName.ToString();
		if (FromStr.StartsWith(TEXT("From")))
			return FName(TEXT("To") + FromStr.Mid(4));
		else
			return FromName;
	})));

	DcAutomationUtils::DumpToLog(ToDatum);

	return true;
}


