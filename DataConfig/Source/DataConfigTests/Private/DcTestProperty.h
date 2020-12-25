#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty.generated.h"

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

UINTERFACE()
class UDcTestInterface1 : public UInterface
{
	GENERATED_BODY()
};

class IDcTestInterface1
{
	GENERATED_BODY()
};

UCLASS()
class UDcTestInterface1Alpha : public UObject, public IDcTestInterface1
{
public:
	GENERATED_BODY()

	UDcTestInterface1Alpha() {}
};

UCLASS()
class UDcTestInterface1Beta : public UObject, public IDcTestInterface1
{
public:
	GENERATED_BODY()

	UDcTestInterface1Beta() {}
};


DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(int, FDcTestDelegate1, int, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDcTestDynMulticastCallback1, int, Value);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FDcTestDynMulticastSparseCallback1, UDcTestDelegateClass1, SparseCallback1, int, Value);

UCLASS()
class UDcTestDelegateClass1 : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION() int ReturnOne(int Int);
	UFUNCTION() void ReturnNone(int Int);

	UPROPERTY() FDcTestDynMulticastSparseCallback1 SparseCallback1;
};

USTRUCT()
struct FDcTestStruct2
{
	GENERATED_BODY()

	//	other scalar types
	UPROPERTY() UClass* ClassField;
	UPROPERTY() TWeakObjectPtr<UObject> WeakObjetField;
	UPROPERTY() TLazyObjectPtr<UObject> LazyObjectField;
	UPROPERTY() TSoftObjectPtr<UObject> SoftObjectField;
	UPROPERTY() TSoftClassPtr<UObject> SoftClassField;
	UPROPERTY() TScriptInterface<IDcTestInterface1> InterfaceField;
	UPROPERTY() TFieldPath<FNameProperty> FieldPathField;

	UPROPERTY() FDcTestDelegate1 DelegateField;
	UPROPERTY() FDcTestDynMulticastCallback1 DynMulticastField;
};

USTRUCT()
struct FDcKeyableStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName Name;
	UPROPERTY(EditAnywhere) int16 Index;

	FORCEINLINE friend uint32 GetTypeHash(const FDcKeyableStruct& In)
	{
		return HashCombine(GetTypeHash(In.Name), GetTypeHash(In.Index));
	}

	FORCEINLINE friend bool operator==(const FDcKeyableStruct& Lhs, const FDcKeyableStruct& Rhs)
	{
		return Lhs.Name == Rhs.Name
			&& Lhs.Index == Rhs.Index;
	}

	FORCEINLINE friend bool operator!=(const FDcKeyableStruct& Lhs, const FDcKeyableStruct& Rhs)
	{
		return Lhs != Rhs;
	}
};


USTRUCT()
struct FDcTestStruct3
{
	GENERATED_BODY()

	//	containers
	UPROPERTY() TArray<FString> StringArray;
	UPROPERTY() TSet<FString> StringSet;
	UPROPERTY() TMap<FString, FString> StringMap;

	//	more containers
	UPROPERTY() TArray<FDcKeyableStruct> StructArray;
	UPROPERTY() TSet<FDcKeyableStruct> StructSet;
	UPROPERTY() TMap<FDcKeyableStruct, FDcKeyableStruct> StructMap;
};


UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UDcBaseShape : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere) FName ShapeName;
};

UCLASS()
class UDcShapeBox : public UDcBaseShape
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere) float Height;
	UPROPERTY(EditAnywhere) float Width;
};

UCLASS()
class UDcShapeSquare : public UDcBaseShape
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere) float Radius;
};


UCLASS()
class UDcTestClass1 : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY() int32 IntField;
	UPROPERTY() FString StrField;
};

USTRUCT()
struct FDcTestStruct4
{
	GENERATED_BODY()

	//	inline object 
	UPROPERTY() UDcBaseShape* InlineObjectField1;
	UPROPERTY() UDcBaseShape* InlineObjectField2;

	//	normal object
	UPROPERTY() UDcTestClass1* NormalObjectField1;
	UPROPERTY() UDcTestClass1* NormalObjectField2;
};


USTRUCT()
struct FDcTestStruct_Blob
{
	GENERATED_BODY()

	UPROPERTY() TArray<uint8> BlobField1;
	UPROPERTY() TArray<int64> BlobField2;

};


