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

	UTEST_OK("FTestUninitializedScriptStructMembersTest roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FTestUninitializedScriptStructMembersTest roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	//	compile checks
	if (0)
	{
		FDcReader Reader;
		TObjectPtr<UDcTestClass1> ObjPtr;
		Reader.ReadTObjectPtr(&ObjPtr).Ok();

		FDcWriter Writer;
		Writer.WriteTObjectPtr(ObjPtr).Ok();
	}

	return true;
}

DC_TEST("DataConfig.Core.Property.FLargeWorldCoordinates")
{
	FVector Source;

	Source.X = 1.5f;
	Source.Y = 1.75f;
	Source.Z = 1.875f;

	FVector Dest;

	FDcPropertyDatum SourceDatum(TBaseStructure<FVector>::Get(), &Source);
	FDcPropertyDatum DestDatum(TBaseStructure<FVector>::Get(), &Dest);

	UTEST_OK("FVector roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FVector roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	UTEST_TRUE("FVector X", Dest.X == 1.5f);
	UTEST_TRUE("FVector Y", Dest.Y == 1.75f);
	UTEST_TRUE("FVector Z", Dest.Z == 1.875f);

	return true;
}

#endif
