#pragma once

//	Rename struct field names while deserializing

#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DcDeserializeRenameStructFieldNames.generated.h"


namespace DcExtra
{

DECLARE_DELEGATE_RetVal_OneParam(FName, FDcExtraRenamer, const FName&);

DATACONFIGEXTRA_API FDcResult DeserializeRenaming(FDcPropertyDatum From, FDcPropertyDatum To, FDcExtraRenamer Renamer);

}	//	namespace DcExtra


USTRUCT()
struct FDcTestExtraRenameFrom1
{
	GENERATED_BODY()

	UPROPERTY() FString FromStr1;
	UPROPERTY() int FromInt1;

	FORCEINLINE friend uint32 GetTypeHash(const FDcTestExtraRenameFrom1& In)
	{
		return HashCombine(GetTypeHash(In.FromStr1), GetTypeHash(In.FromInt1));
	}

	FORCEINLINE friend bool operator==(const FDcTestExtraRenameFrom1& Lhs, const FDcTestExtraRenameFrom1& Rhs)
	{
		return Lhs.FromStr1 == Rhs.FromStr1
			&& Lhs.FromInt1 == Rhs.FromInt1;
	}
};

USTRUCT()
struct FDcTestExtraRenameTo1
{
	GENERATED_BODY()

	UPROPERTY() FString ToStr1;
	UPROPERTY() int ToInt1;

	FORCEINLINE friend uint32 GetTypeHash(const FDcTestExtraRenameTo1& In)
	{
		return HashCombine(GetTypeHash(In.ToStr1), GetTypeHash(In.ToInt1));
	}

	FORCEINLINE friend bool operator==(const FDcTestExtraRenameTo1& Lhs, const FDcTestExtraRenameTo1& Rhs)
	{
		return Lhs.ToStr1 == Rhs.ToStr1
			&& Lhs.ToInt1 == Rhs.ToInt1;
	}
};


USTRUCT()
struct FDcTestExtraRenameFrom2
{
	GENERATED_BODY()

	UPROPERTY() FName FromName1;

	UPROPERTY() TSet<FDcTestExtraRenameFrom1> FromStructSet1;
};

USTRUCT()
struct FDcTestExtraRenameTo2
{
	GENERATED_BODY()

	UPROPERTY() FName ToName1;

	UPROPERTY() TSet<FDcTestExtraRenameTo1> ToStructSet1;
};



