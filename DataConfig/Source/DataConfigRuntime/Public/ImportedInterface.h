#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "ImportedInterface.generated.h"

//	this and classes implement this can't be in the same header
//	ref https://answers.unrealengine.com/questions/844642/cant-place-uinterface-and-its-implementation-class.html?sort=oldest

class UAssetImportData;

//	we'll need something that's marked in the property system 
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class DATACONFIGRUNTIME_API UImportedInterface : public UInterface
{
	GENERATED_BODY()
};

class DATACONFIGRUNTIME_API IImportedInterface
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
public:

	virtual void GetAssetRegistryTags(TArray<UObject::FAssetRegistryTag>& OutTags) const = 0;

	virtual UAssetImportData* GetAssetImportData() const = 0;
	virtual void SetAssetImportData(UAssetImportData* NewAssetImportData) = 0;

#endif // WITH_EDITORONLY_DATA

};

