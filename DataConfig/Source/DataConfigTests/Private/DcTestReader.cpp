#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"

DC_TEST("DataConfig.Core.Reader.Cast")
{
	FDcReader Reader1;
	FDcAnsiJsonReader Reader2;
	FDcWideJsonReader Reader3;
	FDcPropertyReader Reader4;
	FDcPutbackReader Reader5(&Reader2);
	FDcMsgPackReader Reader6;

	{
		FDcReader* Reader = &Reader1;
		UTEST_NOT_NULL("Reader Cast", Reader->CastById<FDcReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcAnsiJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcWideJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPropertyReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPutbackReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcMsgPackReader>());
	}

	{
		FDcReader* Reader = &Reader2;
		UTEST_NULL("Reader Cast", Reader->CastById<FDcReader>());
		UTEST_NOT_NULL("Reader Cast", Reader->CastById<FDcAnsiJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcWideJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPropertyReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPutbackReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcMsgPackReader>());
	}

	{
		FDcReader* Reader = &Reader3;
		UTEST_NULL("Reader Cast", Reader->CastById<FDcReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcAnsiJsonReader>());
		UTEST_NOT_NULL("Reader Cast", Reader->CastById<FDcWideJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPropertyReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPutbackReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcMsgPackReader>());
	}

	{
		FDcReader* Reader = &Reader4;
		UTEST_NULL("Reader Cast", Reader->CastById<FDcReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcAnsiJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcWideJsonReader>());
		UTEST_NOT_NULL("Reader Cast", Reader->CastById<FDcPropertyReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPutbackReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcMsgPackReader>());
	}

	{
		FDcReader* Reader = &Reader5;
		UTEST_NULL("Reader Cast", Reader->CastById<FDcReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcAnsiJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcWideJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPropertyReader>());
		UTEST_NOT_NULL("Reader Cast", Reader->CastById<FDcPutbackReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcMsgPackReader>());
	}

	{
		FDcReader* Reader = &Reader6;
		UTEST_NULL("Reader Cast", Reader->CastById<FDcReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcAnsiJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcWideJsonReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPropertyReader>());
		UTEST_NULL("Reader Cast", Reader->CastById<FDcPutbackReader>());
		UTEST_NOT_NULL("Reader Cast", Reader->CastById<FDcMsgPackReader>());
	}

	return true;
}


