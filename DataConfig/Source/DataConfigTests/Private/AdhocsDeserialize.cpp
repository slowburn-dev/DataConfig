#include "Adhocs.h"
#include "Deserialize/DcDeserializer.h"
#include "Deserialize/DcDeserializeTypes.h"
#include "Deserialize/Converters/StructRootConverter.h"
#include "Deserialize/Converters/PrimitiveConverter.h"
#include "Json/DcJsonReader.h"
#include "Property/DcPropertyWriter.h"

void DeserializeSimple()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	Deserializer.AddConverter(MakeShareable(new FStructRootConverter));
	Deserializer.AddConverter(MakeShareable(new FPrimitiveConverter));

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
	Ctx.Deserializer = &Deserializer;
	Deserializer.Deserialize(Reader, Writer, Ctx);

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





