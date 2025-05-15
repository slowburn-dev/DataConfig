#include "DataConfig/EditorExtra/Editor/DcEditorDumpAssetToLog.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

#include "Engine/Blueprint.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Docking/TabManager.h"

namespace DcEditorExtra
{

TSharedRef<FExtender> DumpAssetToLogExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	for (const FAssetData& Asset : SelectedAssets)
	{
		Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateLambda([Asset](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_DumpToLog", "DataConfig Dump To Log"),
					NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_DumpToLogTooltip", "DataConfig pretty print asset to log"),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateLambda([Asset]{
							if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()))
							{
								//	dump BP class generated class CDO as it makes more sense
								DcAutomationUtils::DumpToLog(FDcPropertyDatum(GetMutableDefault<UObject>(Blueprint->GeneratedClass)));
							}
							else
							{
								DcAutomationUtils::DumpToLog(FDcPropertyDatum(Asset.GetAsset()));
							}

#if ENGINE_MAJOR_VERSION == 5
							FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog"));
#else
	#if ENGINE_MINOR_VERSION >= 26
							FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog"));
	#else
							FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog"));
	#endif
#endif
							}),
							FCanExecuteAction()
						)
					);
		}));
	}


	return Extender;
}

} // namespace DcEditorExtra

