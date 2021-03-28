#include "DcTestDeserializeEditor.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

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
	FDcPropertyDatum DestDatum(FDcEditorExtraTestObjectRefs1::StaticStruct(), &Dest);

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

	FDcPropertyDatum ExpectDatum(FDcEditorExtraTestObjectRefs1::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcEditorExtraTestObjectRefs1", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcEditorExtraTestObjectRefs1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

