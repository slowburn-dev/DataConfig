#include "DataConfigImportFactory.h"
#include "ImportedInterface.h"
#include "EditorFramework/AssetImportData.h"

#define LOCTEXT_NAMESPACE "SLuaImportFactory"

UDataConfigImportFactory::UDataConfigImportFactory()
{
	//	ref: UCSVImportFactory
	bCreateNew = false;
	bEditAfterNew = true;
	//	this is a base pass and it's dynamically resolved at `DoesSupportClass`
	SupportedClass = UDataAsset::StaticClass();	

	bEditorImport = true;
	bText = false;	// this only determines it's char* or TCHAR* read in

	Formats.Add(TEXT("json;JSON files"));
}

FText UDataConfigImportFactory::GetDisplayName() const
{
	return LOCTEXT("DataConfigImportFactoryDescription", "DataConfig Importer");
}

UObject* UDataConfigImportFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	//	ref: CSVImportFactory
	//FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	//	ref: FbxFactory
	UObject* ExistingObject = StaticFindObject(UObject::StaticClass(), InParent, *(InName.ToString()));
	if (ExistingObject)
	{
		//	it's a reimport
		EReimportResult::Type Ret = Reimport(ExistingObject);
		if (Ret == EReimportResult::Succeeded)
		{
			return ExistingObject;
		}
		else
		{
			//	TODO should fallback or prompt to create new asset? 
			return nullptr;
		}
	}

	//UClass* ImportDataClass;
	//	TODO do the deserialization here
	//	peek $type and create the class here dude, because we don't know the type yet, it's self contained.

	return nullptr;
}

bool UDataConfigImportFactory::DoesSupportClass(UClass* Class)
{
	return Class->IsChildOf<UDataAsset>()
		&& Class->ImplementsInterface(UImportedInterface::StaticClass());
}

bool UDataConfigImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension.Compare(TEXT("json"), ESearchCase::IgnoreCase) == 0;
}

bool UDataConfigImportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	//	ref: UReimportCurveTableFactory::CanReimport
	IImportedInterface* Imported = Cast<IImportedInterface>(Obj);
	if (Imported
		&& Imported->GetAssetImportData())
	{
		Imported->GetAssetImportData()->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UDataConfigImportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	IImportedInterface* Imported = Cast<IImportedInterface>(Obj);
	if (Imported
		&& ensure(NewReimportPaths.Num() == 1))
	{
		Imported->GetAssetImportData()->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UDataConfigImportFactory::Reimport(UObject* Obj)
{
	return EReimportResult::Failed;
}

int32 UDataConfigImportFactory::GetPriority() const
{
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE

