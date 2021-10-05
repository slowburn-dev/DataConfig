#include "DcTestProperty3.h"
#include "DcTestCommon.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

#if ENGINE_MAJOR_VERSION == 5

DC_TEST("DataConfig.Core.Property.TObjectPtr")
{
	FTestUninitializedScriptStructMembersTest Source;
	TObjectPtr<UDcTestClass1> Obj = NewObject<UDcTestClass1>();
	Obj->IntField = 123;
	Obj->StrField = TEXT("Foo");

	Source.InitializedObjectReference = Obj;
	Source.UninitializedObjectReference = nullptr;

	FTestUninitializedScriptStructMembersTest Dest;

	FDcPropertyDatum SourceDatum(TBaseStructure<FTestUninitializedScriptStructMembersTest>::Get(), &Source);
	FDcPropertyDatum DestDatum(TBaseStructure<FTestUninitializedScriptStructMembersTest>::Get(), &Dest);

	UTEST_OK("FDcTestStruct4 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStruct4 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	//	compile checks
	if (0)
	{
		FDcReader Reader;
		TObjectPtr<UDcTestClass1> ObjPtr;
		Reader.ReadTObjectPtr(&ObjPtr).Ok();
	}

	return true;
}

#endif
