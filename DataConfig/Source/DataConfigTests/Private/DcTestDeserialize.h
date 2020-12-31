#pragma once

#include "CoreMinimal.h"
#include "DcTestDeserialize.generated.h"

UENUM(meta = (Bitflags))
enum class EDcTestEnumFlag :uint32
{
	None	= 0,
	One		= (1 << 0),
	Two		= (1 << 1),
	Three	= (1 << 2),
	Four	= (1 << 3),
	Five	= (1 << 4),
	Breakdown = (1 << 5),
	Babe	= (1 << 6),
};


USTRUCT()
struct FDcTestStructEnumFlag1
{
	GENERATED_BODY()

	UPROPERTY() EDcTestEnumFlag EnumFlagField1;
	UPROPERTY() EDcTestEnumFlag EnumFlagField2;
};

