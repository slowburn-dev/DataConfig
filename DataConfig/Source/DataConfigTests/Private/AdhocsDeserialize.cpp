#include "Adhocs.h"
#include "Deserialize/DcDeserializer.h"
#include "Deserialize/DcDeserializeTypes.h"
#include "Deserialize/DcDeserializerSetup.h"
#include "Json/DcJsonReader.h"
#include "Property/DcPropertyWriter.h"

void DeserializeSimple()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"AName" : "FromJson",
			"ABool" : true,
			"AStr" : "Wooooooot",
		}
	)");
	Reader.SetNewString(&Str);

	FTestStruct_Alpha OutAlpha;
	FPropertyWriter Writer(FPropertyDatum(FTestStruct_Alpha::StaticStruct(), &OutAlpha));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FTestStruct_Alpha::StaticStruct());
	Deserializer.Deserialize(Ctx);

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPropertyReader PropReader(FPropertyDatum(FTestStruct_Alpha::StaticStruct(), &OutAlpha));
		FPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
		FPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);
		FResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}
}


void DeserializeNestedStruct()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"AName" : "FromJson",
			"AStruct" : {
				"ABool" : true,
				"AStr" : "Nest1",
			},
			"AStruct2" : {
				"ABool" : false,
				"AStr" : "Nest2",
			},
		}
	)");
	Reader.SetNewString(&Str);

	FNestStruct1 OutNest;
	FPropertyWriter Writer(FPropertyDatum(FNestStruct1::StaticStruct(), &OutNest));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FNestStruct1::StaticStruct());
	Deserializer.Deserialize(Ctx);

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPropertyReader PropReader(FPropertyDatum(FNestStruct1::StaticStruct(), &OutNest));
		FPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
		FPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);
		FResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}
}





