#pragma once

#include "CoreMinimal.h"
#include "ImportedInterface.h"
#include "Engine/DataAsset.h"
#include "ImportedDataAsset.generated.h"

UCLASS()
class DATACONFIGRUNTIME_API UImportedDataAsset : public UDataAsset, public IImportedInterface
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
public:
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;

	UAssetImportData* GetAssetImportData() const override;
	void SetAssetImportData(UAssetImportData* NewAssetImportData) override;

	/** Importing data and options used for this asset */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif // WITH_EDITORONLY_DATA

};

UCLASS()
class DATACONFIGRUNTIME_API UPrimaryImportedDataAsset : public UPrimaryDataAsset, public IImportedInterface
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
public:
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;

	UAssetImportData* GetAssetImportData() const override;
	void SetAssetImportData(UAssetImportData* NewAssetImportData) override;

	/** Importing data and options used for this asset */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif // WITH_EDITORONLY_DATA

};


