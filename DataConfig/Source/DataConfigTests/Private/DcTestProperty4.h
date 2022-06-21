#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty.h"
#include "DcTestProperty2.h"
#include "DcTestProperty4.generated.h"

USTRUCT()
struct FDcTestObjectRefs1
{
	GENERATED_BODY()

	//	refs
	UPROPERTY() TArray<UDcTestClass1*> RefObjectArr;
	UPROPERTY() TSet<UDcTestClass1*> RefObjectSet;
	UPROPERTY() TMap<FString, UDcTestClass1*> RefObjectMap;

	//	inline
	UPROPERTY() TArray<UDcBaseShape*> InlineObjectArr;
	UPROPERTY() TSet<UDcBaseShape*> InlineObjectSet;
	UPROPERTY() TMap<FString, UDcBaseShape*> InlineObjectMap;

	void MakeFixture();
};

USTRUCT()
struct FDcTestArrayDim1
{
	GENERATED_BODY()

	//	bool array not allowed
	UPROPERTY() FName NameArr[2];
	UPROPERTY() FString StringArr[3];
	UPROPERTY() FText TextArr[4];

	UPROPERTY() EDcTestEnum1 EnumArr[2];

	UPROPERTY() float FloatArr[2];
	UPROPERTY() double DoubleArr[3];

	UPROPERTY() int8 Int8Arr[2];
	UPROPERTY() int16 Int16Arr[3];
	UPROPERTY() int32 Int32Arr[4];
	UPROPERTY() int64 Int64Arr[5];

	UPROPERTY() uint8 UInt8Arr[2];
	UPROPERTY() uint16 UInt16Arr[3];
	UPROPERTY() uint32 UInt32Arr[4];
	UPROPERTY() uint64 UInt64Arr[5];

	//	array/set/map array not allowed
	UPROPERTY() UDcTestClass1* ObjectRefArr[2];
	UPROPERTY() UDcBaseShape* ObjectInlineArr[3];
	UPROPERTY() UClass* ClassArr[3];

	UPROPERTY() TWeakObjectPtr<UObject> WeakObjectArr[2];
	UPROPERTY() TLazyObjectPtr<UObject> LazyObjectArr[3];
	UPROPERTY() TSoftObjectPtr<UObject> SoftObjectArr[4];

	UPROPERTY() TSoftClassPtr<UObject> SoftClassArr[2];
	UPROPERTY() TScriptInterface<IDcTestInterface1> InterfaceArr[3];
	UPROPERTY() TFieldPath<FNameProperty> FieldPathArr[4];

	UPROPERTY() FDcTestDelegate1 DelegateArr[2];
	UPROPERTY() FDcTestDynMulticastCallback1 DynMulticastArr[3];

	UPROPERTY() FDcTestStructSimple StructArr[2];

	void MakeFixture();
};


UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UDcTestArrayDimInner2 : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY() FName InnerNameArr[2];
	UPROPERTY() int InnerIntArr[2];
};


UCLASS()
class UDcTestArrayDim2 : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY() FName NameArr[2];
	UPROPERTY() FString StringArr[3];

	UPROPERTY() UDcTestArrayDimInner2* InlineObjectArr[2];

	void MakeFixture();
};


USTRUCT()
struct FDcTestHighlight
{
	GENERATED_BODY()

	UPROPERTY() FName NameField;
	UPROPERTY() FName NameArr[2];
	UPROPERTY() TArray<FString> StrArr;
	UPROPERTY() TMap<FString, FString> StrMap;
	UPROPERTY() TSet<FString> StrSet;

	void MakeFixture();
};

USTRUCT()
struct FDcTestScalarArrayBlob
{
	GENERATED_BODY()

	UPROPERTY() FColor ColorField;
	UPROPERTY() FColor ColorArr[2];
};


USTRUCT()
struct FDcTestSetMapOfName
{
	GENERATED_BODY()

	UPROPERTY() TSet<FName> NameSet;
	UPROPERTY() TMap<FName, FString> NameStrMap;
};


