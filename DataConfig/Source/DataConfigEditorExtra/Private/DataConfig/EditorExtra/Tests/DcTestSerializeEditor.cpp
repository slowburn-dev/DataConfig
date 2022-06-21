#include "DcTestSerDeEditor.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonWriter.h"

DC_TEST("DataConfig.EditorExtra.Serialize.ObjectReference2")
{
	FDcEditorExtraTestObjectRefs1 Value;

	UDcEditorExtraNativeDataAsset* DataAsset = Cast<UDcEditorExtraNativeDataAsset>(StaticLoadObject(
		UDcEditorExtraNativeDataAsset::StaticClass(),
		nullptr,
		TEXT("/DataConfig/DcFixture/DcTestNativeDataAssetAlpha"),
		nullptr
	));

	Value.ObjField1 = DataAsset;
	Value.ObjField2 = nullptr;
	Value.ObjField3 = nullptr;
	Value.ObjField4 = nullptr;

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		{
			"ObjField1" : "DcEditorExtraNativeDataAsset'/DataConfig/DcFixture/DcTestNativeDataAssetAlpha.DcTestNativeDataAssetAlpha'",
			"ObjField2" : null,
			"ObjField3" : null,
			"ObjField4" : null
		}

	)"));

	UTEST_OK("Serialize FDcEditorExtraTestObjectRefs1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcEditorExtraTestObjectRefs1 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}




