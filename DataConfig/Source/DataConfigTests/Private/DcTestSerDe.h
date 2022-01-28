#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty.h"
#include "Templates/SubclassOf.h"
#include "DcTestSerDe.generated.h"

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

	void MakeFixture();
};

USTRUCT()
struct FDcTestStructShapeContainer1
{
	GENERATED_BODY()

	UPROPERTY() UDcBaseShape* ShapeField1;
	UPROPERTY() UDcBaseShape* ShapeField2;
	UPROPERTY() UDcBaseShape* ShapeField3;

	void MakeFixtureEmpty();
	void MakeFixture();
};

USTRUCT()
struct FDcTestStructObjectRef1
{
	GENERATED_BODY()

	UPROPERTY() UObject* ObjField1;
	UPROPERTY() UObject* ObjField2;
	UPROPERTY() UObject* ObjField3;
};

USTRUCT()
struct FDcTestStructSubClass1
{
	GENERATED_BODY()

	UPROPERTY() TSubclassOf<UStruct> StructSubClassField1;
	UPROPERTY() TSubclassOf<UStruct> StructSubClassField2;
	UPROPERTY() TSubclassOf<UStruct> StructSubClassField3;
};

USTRUCT()
struct FDcTestStructRefs1
{
	GENERATED_BODY()

	UPROPERTY() UObject* ObjectField1;
	UPROPERTY() UObject* ObjectField2;

	UPROPERTY() TSoftObjectPtr<UObject> SoftField1;
	UPROPERTY() TSoftObjectPtr<UObject> SoftField2;

	UPROPERTY() TWeakObjectPtr<UObject> WeakField1;
	UPROPERTY() TWeakObjectPtr<UObject> WeakField2;

	UPROPERTY() TLazyObjectPtr<UObject> LazyField1;
	UPROPERTY() TLazyObjectPtr<UObject> LazyField2;

	void MakeFixture();
};


USTRUCT()
struct FDcTestStructRefs2
{
	GENERATED_BODY()

	UPROPERTY() UClass* RawClassField1;
	UPROPERTY() UClass* RawClassField2;

	UPROPERTY() TSubclassOf<UObject> SubClassField1;
	UPROPERTY() TSubclassOf<UObject> SubClassField2;

	UPROPERTY() TSoftClassPtr<UObject> SoftClassField1;
	UPROPERTY() TSoftClassPtr<UObject> SoftClassField2;

	void MakeFixture();
};




