#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.h"

#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilityBlueprint.h"
#include "Textures/SlateIcon.h"
#include "ToolMenuSection.h"
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

FText FAssetTypeActions_DcGameplayAbility::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DcGameplayAbility", "DataConfig GameplayAbility"); 
}

FColor FAssetTypeActions_DcGameplayAbility::GetTypeColor() const
{
	return FColor(201, 29, 85); 
}

uint32 FAssetTypeActions_DcGameplayAbility::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FAssetTypeActions_DcGameplayAbility::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UGameplayAbilityBlueprint>> Abilities = GetTypedWeakObjectPtrs<UGameplayAbilityBlueprint>(InObjects);

	if (Abilities.Num() != 1)
		return;

	TWeakObjectPtr<UGameplayAbilityBlueprint> AbilityBP = Abilities[0];
	if (!AbilityBP.IsValid())
		return;

	UGameplayAbility* AbilityCDO = CastChecked<UGameplayAbility>(AbilityBP->GeneratedClass->ClassDefaultObject);
	
	Section.AddMenuEntry(
		TEXT("DcEditorExtra_LoadFromJson"),
		NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadFromJson", "Load From JSON"), 
		NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadFromJsonTooltip", "Load default values from a JSON file"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=]
			{
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
					
					DeserializeGameplayAbility(AbilityCDO, Reader);
				}
			}),
			FCanExecuteAction()
		)
	);
}

bool FAssetTypeActions_DcGameplayAbility::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

UClass* FAssetTypeActions_DcGameplayAbility::GetSupportedClass() const
{
	return UGameplayAbilityBlueprint::StaticClass();
}

FText FAssetTypeActions_DcGameplayEffect::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DcGameplayEffect", "DataConfig GameplayEffect"); 
}

FColor FAssetTypeActions_DcGameplayEffect::GetTypeColor() const
{
	return FColor(201, 29, 85); 
}

uint32 FAssetTypeActions_DcGameplayEffect::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FAssetTypeActions_DcGameplayEffect::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
}

bool FAssetTypeActions_DcGameplayEffect::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

UClass* FAssetTypeActions_DcGameplayEffect::GetSupportedClass() const
{
	return UGameplayEffect::StaticClass();
}

} // namespace DcEditorExtra
