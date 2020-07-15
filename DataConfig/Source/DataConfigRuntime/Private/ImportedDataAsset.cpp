#include "ImportedDataAsset.h"
#include "EditorFramework/AssetImportData.h"

#if WITH_EDITORONLY_DATA
void UImportedDataAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	//	need this for re import to work.
	//	this would populate FAssetData::TagsAndValues so that `FAssetSourceFilenameCache` can find this
	//	upon reimport
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

UAssetImportData* UImportedDataAsset::GetAssetImportData() const
{
	return AssetImportData;
}

void UImportedDataAsset::SetAssetImportData(UAssetImportData* NewAssetImportData)
{
	check(NewAssetImportData);
	check(AssetImportData == nullptr);
	AssetImportData = NewAssetImportData;
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITORONLY_DATA
//	TODO this is totally identical to above
void UPrimaryImportedDataAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

UAssetImportData* UPrimaryImportedDataAsset::GetAssetImportData() const
{
	return AssetImportData;
}

void UPrimaryImportedDataAsset::SetAssetImportData(UAssetImportData* NewAssetImportData)
{
	check(NewAssetImportData);
	check(AssetImportData == nullptr);
	AssetImportData = NewAssetImportData;
}
#endif // WITH_EDITORONLY_DATA
