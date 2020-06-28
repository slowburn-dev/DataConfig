#include "Adhocs.h"
#include "Deserialize/DcDeserializer.h"
#include "Deserialize/DcDeserializeTypes.h"
#include "Deserialize/Converters/DcPrimitiveDeserializers.h"
#include "Deserialize/Converters/DcStructDeserializers.h"
#include "Json/DcJsonReader.h"
#include "Property/DcPropertyWriter.h"

void DeserializeSimple()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	Deserializer.AddDirectHandler(UBoolProperty::StaticClass(), FDeserializeDelegate::CreateStatic(BoolDeserializeHandler));
	Deserializer.AddDirectHandler(UNameProperty::StaticClass(), FDeserializeDelegate::CreateStatic(NameDeserializeHandler));
	Deserializer.AddDirectHandler(UStrProperty::StaticClass(), FDeserializeDelegate::CreateStatic(StringDeserializeHandler));
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDeserializeDelegate::CreateStatic(StructRootDeserializeHandler));

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





