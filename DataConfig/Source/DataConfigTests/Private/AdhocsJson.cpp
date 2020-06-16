#include "Adhocs.h"
#include "UObject/UnrealType.h"
#include "DcTypes.h"
#include "DcErrorCodes.h"
#include "Json/DcJsonReader.h"

using namespace DataConfig;

void JsonReader1()
{
	FJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"Hello" : "JSON",
			"Truthy" : true,
			"Falsy" : false,
		}

	)");
	Reader.SetNewString(&Str);


	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
		FPipeVisitor PrettyPrintVisit(&Reader, &Writer);
		FResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}


}
