#include "DataConfig/EditorExtra/Deserialize/DcDeserializeBPClassInstance.h"
#include "DataConfig/Automation/DcAutomation.h"

namespace DcEditorExtra {

EDcDeserializePredicateResult PredicateIsBPSubObjectProperty(FDcDeserializeContext& Ctx)
{
	if (Ctx.TopProperty().IsUObject())
		return EDcDeserializePredicateResult::Pass;


	return EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerBPSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return DcOk();
}

} // namespace DcEditorExtra

FString UDcTestInstancedBPClassBase::GetClassID_Implementation()
{
	return TEXT("UBPClassBase");
}

DC_TEST("DataConfig.EditorExtra.BlueprintClassSubObject")
{

	FString Str = TEXT(R"(
		{
			"TagContainerField1" : [],
			"TagContainerField2" : [
				"DataConfig.Foo.Bar",
				"DataConfig.Foo.Bar.Baz",
				"DataConfig.Tar.Taz",
			]
		}
	)");


	return true;
}

