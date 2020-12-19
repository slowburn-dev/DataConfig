#pragma once

#include "CoreMinimal.h"

#include "DcTestCommon.generated.h"

UENUM()
enum class EDcTestEnum1 : int64
{
	Foo,
	Bar,
	Tard,
};

USTRUCT()
struct FDcTestStruct1
{
	GENERATED_BODY();

	// basic scalar types
	UPROPERTY() bool BoolField;
	UPROPERTY() FName NameField;
	UPROPERTY() FString StringField;
	UPROPERTY() FText TextField;
	UPROPERTY() EDcTestEnum1 EnumField;

	UPROPERTY() float FloatField;
	UPROPERTY() double DoubleField;

	UPROPERTY() int8 Int8Field;
	UPROPERTY() int16 Int16Field;
	UPROPERTY() int32 Int32Field;
	UPROPERTY() int64 Int64Field;

	UPROPERTY() uint8 UInt8Field;
	UPROPERTY() uint16 UInt16Field;
	UPROPERTY() uint32 UInt32Field;
	UPROPERTY() uint64 UInt64Field;

};





