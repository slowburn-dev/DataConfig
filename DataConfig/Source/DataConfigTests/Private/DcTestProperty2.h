#pragma once

#include "CoreMinimal.h"
#include "Math/NumericLimits.h"
#include "DcTestProperty2.generated.h"

USTRUCT()
struct FDcTestStructSimple
{
	GENERATED_BODY()
	UPROPERTY() FName NameField;
	UPROPERTY() FString StrField;
};

USTRUCT()
struct FDcTestStructNest1
{
	GENERATED_BODY()
	UPROPERTY() FName NameField;
	UPROPERTY() FDcTestStructSimple StructField;
};

USTRUCT()
struct FDcTestStructNest2
{
	GENERATED_BODY()
	UPROPERTY() TMap<FString, FDcTestStructNest1> StrStructMapField;
};

USTRUCT()
struct FDcTestStructNest3
{
	GENERATED_BODY()
	UPROPERTY() FDcTestStructNest2 StructField;
};

USTRUCT()
struct FDcTestStructNest4
{
	GENERATED_BODY()
	UPROPERTY() TArray<FDcTestStructNest3> StructArrayField;
};

UENUM()
enum class EDcTestEnum_UInt8 : uint8
{
	Zero = 0,
	Max = TNumericLimits<uint8>::Max(),
};

UENUM()
enum class EDcTestEnum_UInt16 : uint16
{
	Zero = 0,
	Max = TNumericLimits<uint16>::Max(),
};

UENUM()
enum class EDcTestEnum_UInt32 : uint32
{
	Zero = 0,
	Max = TNumericLimits<uint32>::Max(),
};

UENUM()
enum class EDcTestEnum_UInt64 : uint64
{
	Zero = 0,
	Max = TNumericLimits<uint64>::Max(),
};

UENUM()
enum class EDcTestEnum_Int8 : int8
{
	Zero = 0,
	Min = TNumericLimits<int8>::Min(),
	Max = TNumericLimits<int8>::Max(),
};

UENUM()
enum class EDcTestEnum_Int16 : int16
{
	Zero = 0,
	Min = TNumericLimits<int16>::Min(),
	Max = TNumericLimits<int16>::Max(),
};

UENUM()
enum class EDcTestEnum_Int32 : int32
{
	Zero = 0,
	Min = TNumericLimits<int32>::Min(),
	Max = TNumericLimits<int32>::Max(),
};

UENUM()
enum class EDcTestEnum_Int64 : int64
{
	Zero = 0,
	Min = TNumericLimits<int64>::Min(),
	Max = TNumericLimits<int64>::Max(),
};

UENUM()
enum class EDcTestEnum_Flag
{
	Zero	= 0,
	Alpha	= 0x1 << 0,
	Beta	= 0x1 << 1,
	Gamma	= 0x1 << 2,
};
ENUM_CLASS_FLAGS(EDcTestEnum_Flag);


USTRUCT()
struct FDcTestStructEnum1
{
	GENERATED_BODY()

	UPROPERTY() EDcTestEnum_UInt8 EnumUInt8Field;
	UPROPERTY() EDcTestEnum_UInt16 EnumUInt16Field;
	UPROPERTY() EDcTestEnum_UInt32 EnumUInt32Field;
	UPROPERTY() EDcTestEnum_UInt64 EnumUInt64Field;

	UPROPERTY() EDcTestEnum_Int8 EnumInt8Field;
	UPROPERTY() EDcTestEnum_Int16 EnumInt16Field;
	UPROPERTY() EDcTestEnum_Int32 EnumInt32Field;
	UPROPERTY() EDcTestEnum_Int64 EnumInt64Field;

	UPROPERTY() EDcTestEnum_Flag EnumFlagField;
};

USTRUCT()
struct FDcTestStructDefaultValue1
{
	GENERATED_BODY()

	UPROPERTY() int IntField1;
	UPROPERTY() int IntField2 = 123;
	UPROPERTY() int IntField3 = 234;
};

USTRUCT()
struct FDcTestStructDefaultValue2
{
	GENERATED_BODY()

	UPROPERTY() TArray<FDcTestStructDefaultValue1> StructsArray;
	UPROPERTY() TMap<FName, FDcTestStructDefaultValue1> StructsMap;
};




