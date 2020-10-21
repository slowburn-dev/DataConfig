#include "Adhocs.h"
#include "UObject/UnrealType.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"

void JsonReader1()
{
	FDcJsonReader Reader;
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
		FDcPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
		FDcPipeVisitor PrettyPrintVisit(&Reader, &Writer);
		FDcResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}


}
