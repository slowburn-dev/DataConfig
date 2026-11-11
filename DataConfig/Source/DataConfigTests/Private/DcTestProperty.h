#pragma once

#include "CoreMinimal.h"
#include "IDcTestInterface.h"
#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"
#include "DcTestProperty.generated.h"

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
	GENERATED_BODY()

	// basic scalar types
	UPROPERTY() bool BoolField = false;
	UPROPERTY() FName NameField;
	UPROPERTY() FString StringField;
	UPROPERTY() FText TextField;
	UPROPERTY() EDcTestEnum1 EnumField = EDcTestEnum1::Foo;

	UPROPERTY() float FloatField = 0;
	UPROPERTY() double DoubleField = 0;

	UPROPERTY() int8 Int8Field = 0;
	UPROPERTY() int16 Int16Field = 0;
	UPROPERTY() int32 Int32Field = 0;
	UPROPERTY() int64 Int64Field = 0;

	UPROPERTY() uint8 UInt8Field = 0;
	UPROPERTY() uint16 UInt16Field = 0;
	UPROPERTY() uint32 UInt32Field = 0;
	UPROPERTY() uint64 UInt64Field = 0;

	void MakeFixture();
};


DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(int, FDcTestDelegate1, int, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDcTestDynMulticastCallback1, int, Value);

//	note that sparse callback is bound to a UClass
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
	UPROPERTY() UClass* ClassField = nullptr;
	UPROPERTY() TWeakObjectPtr<UObject> WeakObjetField;
	UPROPERTY() TSoftObjectPtr<UObject> SoftObjectField;
	UPROPERTY() TSoftClassPtr<UObject> SoftClassField;
	UPROPERTY() TScriptInterface<IDcTestInterface1> InterfaceField;
	UPROPERTY() TFieldPath<FProperty> FieldPathField;

	UPROPERTY() FDcTestDelegate1 DelegateField;
	UPROPERTY() FDcTestDynMulticastCallback1 DynMulticastField;
};

USTRUCT()
struct FDcKeyableStruct
{
	GENERATED_BODY()

	UPROPERTY() FName Name;
	UPROPERTY() int16 Index = 0;

	FORCEINLINE friend uint32 GetTypeHash(const FDcKeyableStruct& In)
	{
		return HashCombine(GetTypeHash(In.Name), GetTypeHash(In.Index));
	}

	FORCEINLINE friend bool operator==(const FDcKeyableStruct& Lhs, const FDcKeyableStruct& Rhs)
	{
		return Lhs.Name == Rhs.Name
			&& Lhs.Index == Rhs.Index;
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

	void MakeFixtureNoStructMap();
	void MakeFixtureFull();
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
	DCEXTRA_ZEROINIT_CONSTRUCTOR(FDcTestStruct4)

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


