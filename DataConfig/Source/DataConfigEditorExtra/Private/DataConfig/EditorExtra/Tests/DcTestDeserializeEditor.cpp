#include "DcTestSerDeEditor.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Json/DcJsonReader.h"

DC_TEST("DataConfig.EditorExtra.Deserialize.ObjectReference2")
{
	FString Str = TEXT(R"(
		{
			"ObjField1" : "DcEditorExtraNativeDataAsset'/DataConfig/DcFixture/DcTestNativeDataAssetAlpha.DcTestNativeDataAssetAlpha'",
			"ObjField2" : "/DataConfig/DcFixture/DcTestNativeDataAssetAlpha",
			"ObjField3" : 
			{
				"$type" : "DcEditorExtraNativeDataAsset",
				"$path" : "/DataConfig/DcFixture/DcTestNativeDataAssetAlpha"
			},
			"ObjField4" : null,
		}
	)");
	FDcJsonReader Reader(Str);

	FDcEditorExtraTestObjectRefs1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FDcEditorExtraTestObjectRefs1 Expect;

	UDcEditorExtraNativeDataAsset* DataAsset = Cast<UDcEditorExtraNativeDataAsset>(StaticLoadObject(
		UDcEditorExtraNativeDataAsset::StaticClass(),
		nullptr,
		TEXT("/DataConfig/DcFixture/DcTestNativeDataAssetAlpha"),
		nullptr
	));

	Expect.ObjField1 = DataAsset;
	Expect.ObjField2 = DataAsset;
	Expect.ObjField3 = DataAsset;
	Expect.ObjField4 = nullptr;

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcEditorExtraTestObjectRefs1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcEditorExtraTestObjectRefs1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

