#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty.h"
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
ENUM_CLASS_FLAGS(EDcTestEnumFlag);


USTRUCT()
struct FDcTestStructEnumFlag1
{
	GENERATED_BODY()

	UPROPERTY() EDcTestEnumFlag EnumFlagField1;
	UPROPERTY() EDcTestEnumFlag EnumFlagField2;
};

USTRUCT()
struct FDcTestStructShapeContainer1
{
	GENERATED_BODY()

	UPROPERTY() UDcBaseShape* ShapeField1;
	UPROPERTY() UDcBaseShape* ShapeField2;
	UPROPERTY() UDcBaseShape* ShapeField3;
};




