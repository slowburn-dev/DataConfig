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
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayTags.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"

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
		FDcDeserializePredicate::CreateStatic(PredicateIsGameplayAttribute),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayAttributeDeserialize)
	);
	GameplayAbilityDeserializer->AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTag),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagDeserialize)
	);
	GameplayAbilityDeserializer->AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTagContainer),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagContainerDeserialize)
	);
}

EDcDeserializePredicateResult PredicateIsGameplayAttribute(FDcDeserializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == FGameplayAttribute::StaticStruct()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerGameplayAttributeDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	FString AttributeStr;
	DC_TRY(Ctx.Reader->ReadString(&AttributeStr));

	int32 Ix;
	bool bFound = AttributeStr.FindChar(TCHAR('.'), Ix);
	if (!bFound)
		return DC_FAIL(DcDEditorExtra, InvalidGameplayAttribute) << AttributeStr;
	
	FStringView View =  AttributeStr;
	FString Head = FString(View.Left(Ix));
	FString Tail = FString(View.RightChop(Ix + 1));
	
	UClass* AttributeClass = FindObject<UClass>(ANY_PACKAGE, *Head, true);	
	if (AttributeClass == nullptr)
		return DC_FAIL(DcDDeserialize, UObjectByNameNotFound) << TEXT("Class") << Head;

	FProperty* AttributeProperty = DcPropertyUtils::FindEffectivePropertyByName(AttributeClass, *Tail);
	if (AttributeProperty == nullptr)
		return DC_FAIL(DcDReadWrite, CantFindPropertyByName) << Tail;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FGameplayAttribute* Attribute = (FGameplayAttribute*)Datum.DataPtr;
	Attribute->SetUProperty(AttributeProperty);
	
	return DcOkWithProcessed(OutRet);
}

FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcReader& Reader)
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

FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, FDcReader& Reader)
{
	LazyInitializeDeserializer();

	FDcPropertyWriter Writer(FDcPropertyDatum(UGameplayEffect::StaticClass(), Instance));

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &GameplayAbilityDeserializer.GetValue();
	Ctx.Properties.Push(UGameplayEffect::StaticClass());
	DC_TRY(Ctx.Prepare());
	
	return GameplayAbilityDeserializer->Deserialize(Ctx);
}

static FDcResult SelectJSONAndLoadIntoBlueprintCDO(FAssetData Asset, TFunctionRef<FDcResult(UBlueprint*, FDcReader& Reader)> DeserializeFunc)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
		return DcOk(); // silently fail on non desktop

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
		return DcOk();	// cancel is OK

	check(OpenFilenames.Num() == 1);
	FString Filename = OpenFilenames.Pop();

	FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_IMPORT, FPaths::GetPath(Filename));

	FString JsonStr;
	bool bLoadFile = FFileHelper::LoadFileToString(JsonStr, *Filename, FFileHelper::EHashOptions::None);
	if (!bLoadFile)
		return DC_FAIL(DcDEditorExtra, LoadFileByPathFail) << Filename;

	FDcJsonReader Reader;
	Reader.SetNewString(*JsonStr);
	Reader.DiagFilePath = MoveTemp(Filename);

	UBlueprint* Blueprint = CastChecked<UBlueprint>(Asset.GetAsset());

	//  regenerate CDO before deserializing from JSON to avoid stale values from last run
	Blueprint->GeneratedClass->PurgeClass(false);
	FCompilerResultsLog Results;
	FKismetCompilerOptions CompileOptions;
	FKismetCompilerContext Compiler(Blueprint, Results, CompileOptions);
	Compiler.Compile();

	if (Results.NumErrors > 0)
		return DC_FAIL(DcDEditorExtra, KismetCompileFail) << Blueprint->GetFriendlyName();

	DC_TRY(DeserializeFunc(Blueprint, Reader));

	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);

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

		Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateLambda([Asset](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_DumpToLog", "Dump To Log"), 
					NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_DumpToLogTooltip", "Dump Blueprint CDO content to log"),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateLambda([Asset]{

							UBlueprint* Blueprint = CastChecked<UBlueprint>(Asset.GetAsset());
							DcAutomationUtils::DumpToLog(FDcPropertyDatum(Blueprint->GeneratedClass->ClassDefaultObject));
							FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog"));
							
							}),
							FCanExecuteAction()
						)
					);
		}));
		
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

								FDcResult Ret = SelectJSONAndLoadIntoBlueprintCDO(Asset, [](UBlueprint* Blueprint, FDcReader& Reader)
								{
									UGameplayAbility* AbilityCDO = CastChecked<UGameplayAbility>(Blueprint->GeneratedClass->ClassDefaultObject);
									return DeserializeGameplayAbility(AbilityCDO, Reader);
								});

								if (!Ret.Ok())
								{
									DcEnv().FlushDiags();

									FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
									MessageLogModule.OpenMessageLog(TEXT("DataConfig"));
								}
							}),
							FCanExecuteAction()
						)
					);
			}));
			
			break;
		}
		else if (NativeParentClass->IsChildOf(UGameplayEffect::StaticClass()))
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

								FDcResult Ret = SelectJSONAndLoadIntoBlueprintCDO(Asset, [](UBlueprint* Blueprint, FDcReader& Reader)
								{
									UGameplayEffect* EffectCDO = CastChecked<UGameplayEffect>(Blueprint->GeneratedClass->ClassDefaultObject);
									return DeserializeGameplayEffect(EffectCDO, Reader);
								});

								if (!Ret.Ok())
								{
									DcEnv().FlushDiags();

									FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
									MessageLogModule.OpenMessageLog(TEXT("DataConfig"));
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
