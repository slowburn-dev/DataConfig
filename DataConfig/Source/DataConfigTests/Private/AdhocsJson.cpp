#include "Adhocs.h"
#include "UObject/UnrealType.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/DcErrorCodes.h"
#include "DataConfig/Json/DcJsonReader.h"

using namespace DataConfig;

void JsonReader1()
{
	FJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"Hello" : "JSON",
			"Truthy" : true,
			"Falsy" : false,
			"Nest1" : {
				"Nest1" : "yeah",
			},
			"Nest2" : {
				"Nest2" : "wow",
			},
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
