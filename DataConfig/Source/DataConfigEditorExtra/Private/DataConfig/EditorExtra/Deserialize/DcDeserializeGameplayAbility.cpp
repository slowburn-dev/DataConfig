#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.h"

#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "Abilities/GameplayAbility.h"
#include "MessageLogModule.h"
#include "Textures/SlateIcon.h"
#include "ToolMenuSection.h"
#include "DataConfig/DcEnv.h"
#include "Misc/FileHelper.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayTags.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcEditorExtra
{

static TOptional<FDcDeserializer> GameplayAbilityDeserializer;

static void LazyInitializeDeserializer()
{
	if (GameplayAbilityDeserializer.IsSet())
		return;

	GameplayAbilityDeserializer.Emplace();
	DcSetupJsonDeserializeHandlers(GameplayAbilityDeserializer.GetValue());

	GameplayAbilityDeserializer->AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTag),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagDeserialize)
	);
	GameplayAbilityDeserializer->AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTagContainer),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagContainerDeserialize)
	);
}

FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcJsonReader& Reader)
{
	LazyInitializeDeserializer();

	FDcPropertyWriter Writer(FDcPropertyDatum(UGameplayAbility::StaticClass(), Instance));

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &GameplayAbilityDeserializer.GetValue();
	Ctx.Properties.Push(UGameplayAbility::StaticClass());
	DC_TRY(Ctx.Prepare());
	
	return GameplayAbilityDeserializer->Deserialize(Ctx);
}

FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, const TCHAR* Str)
{
	return DcOk();
}

TSharedRef<FExtender> GameplayAbilityEffectExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	for (auto& Asset : SelectedAssets)
	{
		FString ParentClassPath;
		if(!Asset.GetTagValue(FBlueprintTags::NativeParentClassPath, ParentClassPath))
			continue;

		UObject* Outer = nullptr;
		ResolveName( Outer, ParentClassPath, false, false );
		UClass* NativeParentClass = FindObject<UClass>( ANY_PACKAGE, *ParentClassPath );

		if (NativeParentClass->IsChildOf(UGameplayAbility::StaticClass()))
		{
			Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, TSharedPtr<FUICommandList>(),
				FMenuExtensionDelegate::CreateLambda([Asset](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
						NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadFromJson", "Load From JSON"), 
						NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadFromJsonTooltip", "Load default values from a JSON file"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([Asset]{
								IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
								if (DesktopPlatform)
								{
									const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
									TArray<FString> OpenFilenames;
									int32 FilterIndex = -1;
									bool bSelected = DesktopPlatform->OpenFileDialog(
										ParentWindowWindowHandle,
										TEXT("Select JSON File"),
										FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
										TEXT(""),
										TEXT("Json config file (*.json)|*.json"),
										EFileDialogFlags::None,
										OpenFilenames,
										FilterIndex
									);

									if (!bSelected)
										return;

									check(OpenFilenames.Num() == 1);
									FString Filename = OpenFilenames.Pop();

									FString JsonStr;
									bool bLoadFile = FFileHelper::LoadFileToString(JsonStr, *Filename, FFileHelper::EHashOptions::None);
									if (!bLoadFile)
										return;

									FDcJsonReader Reader;
									Reader.SetNewString(*JsonStr);
									Reader.DiagFilePath = MoveTemp(Filename);

									UBlueprint* AbilityBP = CastChecked<UBlueprint>(Asset.GetAsset());
									UGameplayAbility* AbilityCDO = CastChecked<UGameplayAbility>(AbilityBP->GeneratedClass->ClassDefaultObject);
									
									if (!DeserializeGameplayAbility(AbilityCDO, Reader).Ok())
									{
										DcEnv().FlushDiags();

										FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
										MessageLogModule.OpenMessageLog(TEXT("DataConfig"));
									}
								}
							}),
							FCanExecuteAction()
						)
					);
				}));
			
			break;
		}
	}

    return Extender;
}

} // namespace DcEditorExtra
