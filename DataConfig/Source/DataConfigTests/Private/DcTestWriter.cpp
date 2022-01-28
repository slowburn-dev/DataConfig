#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Writer/DcNoopWriter.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "DataConfig/Writer/DcWeakCompositeWriter.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"

DC_TEST("DataConfig.Core.Writer.Cast")
{
	FDcWriter Writer1;
	FDcNoopWriter Writer2;
	FDcPrettyPrintWriter Writer3;
	FDcWeakCompositeWriter Writer4;
	FDcAnsiJsonWriter Writer5;
	FDcWideJsonWriter Writer6;
	FDcMsgPackWriter Writer7;
	FDcPropertyWriter Writer8;

	{
		FDcWriter* Writer = &Writer1;
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	{
		FDcWriter* Writer = &Writer2;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	{
		FDcWriter* Writer = &Writer3;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	{
		FDcWriter* Writer = &Writer4;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}


	{
		FDcWriter* Writer = &Writer5;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	{
		FDcWriter* Writer = &Writer6;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	{
		FDcWriter* Writer = &Writer7;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	{
		FDcWriter* Writer = &Writer8;
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcNoopWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcPrettyPrintWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWeakCompositeWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcAnsiJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcWideJsonWriter>());
		UTEST_NULL("Writer Cast", Writer->CastById<FDcMsgPackWriter>());
		UTEST_NOT_NULL("Writer Cast", Writer->CastById<FDcPropertyWriter>());
	}

	return true;
}




