#pragma once

//	Deserialize struct while renaming fields according to `Renamer`

#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DcDeserializeRenaming.generated.h"


namespace DcExtra
{

DECLARE_DELEGATE_RetVal_OneParam(FName, FDcExtraRenamer, const FName&);

DATACONFIGEXTRA_API FDcResult DeserializeRenaming(FDcPropertyDatum From, FDcPropertyDatum To, FDcExtraRenamer Renamer);

}	//	namespace DcExtra


USTRUCT()
struct FDcTestExtraRenameFrom1
{
	GENERATED_BODY()

	UPROPERTY() FName FromName1;

	UPROPERTY() TArray<int> FromIntArray1;
};

USTRUCT()
struct FDcTestExtraRenameTo1
{
	GENERATED_BODY()

	UPROPERTY() FName ToName1;

	UPROPERTY() TArray<int> ToIntArray1;
};



