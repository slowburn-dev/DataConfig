#pragma once

#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "Templates/IsEnum.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "Adhocs.generated.h"

USTRUCT()
struct FTestStruct_Alpha
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) bool ABool;
	UPROPERTY(EditAnywhere) FString AStr;

	UPROPERTY(EditAnywhere) TArray<FName> Names;
};

USTRUCT()
struct FStructWithSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TSet<FName> ASet;
};

UCLASS()
class UTestObj_Alpha : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere) FString AStr;
	UPROPERTY(VisibleAnywhere) FTestStruct_Alpha AStruct;
};


USTRUCT()
struct FTestStruct_ObjRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced) UTestObj_Alpha* AlphaRef;
	UPROPERTY(EditAnywhere, Instanced) UTestObj_Alpha* BetaRef;

};


USTRUCT()
struct FNestStruct1
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) FTestStruct_Alpha AStruct;
	UPROPERTY(EditAnywhere) FTestStruct_Alpha AStruct2;
};

USTRUCT()
struct FTestStruct_OldSchool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) bool ABool;
	UPROPERTY(EditAnywhere) FString AStr;
};

USTRUCT()
struct FNestStruct_OldSchool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) FTestStruct_OldSchool AStruct;
};

USTRUCT()
struct FMapContainer1
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TMap<FString, FString> StrStrMap;
	UPROPERTY(EditAnywhere) TMap<FName, bool> NameBoolMap;

};

USTRUCT()
struct FMapOfStruct1
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TMap<FString, FTestStruct_Alpha> StrStructMap;
};

USTRUCT()
struct FKeyableStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName Name;
	UPROPERTY(EditAnywhere) bool Index;

	//	inline friend declaration
	//	https://devdocs.io/cpp/language/friend see #2
	FORCEINLINE friend uint32 GetTypeHash(const FKeyableStruct& In)
	{
		return HashCombine(GetTypeHash(In.Name), GetTypeHash(In.Index));
	}

	//	even concept checked this, still need these 2 operators to be put into struct
	FORCEINLINE friend bool operator==(const FKeyableStruct& Lhs, const FKeyableStruct& Rhs)
	{
		return Lhs.Name == Rhs.Name
			&& Lhs.Index == Rhs.Index;
	}

	FORCEINLINE friend bool operator!=(const FKeyableStruct& Lhs, const FKeyableStruct& Rhs)
	{
		return Lhs != Rhs;
	}
};

USTRUCT()
struct FMapOfStruct2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TMap<FKeyableStruct, FTestStruct_Alpha> StructStructMap;
};

UCLASS()
class UTestClass_Alpha : public UObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) bool ABool;
	UPROPERTY(EditAnywhere) FString AStr;

	UPROPERTY(EditAnywhere) FTestStruct_Alpha AStruct;
};

USTRUCT()
struct FObjReference
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) UObject* Obj1;
	UPROPERTY(EditAnywhere) UObject* Obj2;
	UPROPERTY(EditAnywhere) UObject* Obj3;
};

UCLASS()
class UEmptyObject : public UObject
{
	GENERATED_BODY()
};

USTRUCT()
struct FEmptyStruct
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UBaseShape : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere) FName ShapeName;
};

UCLASS()
class UShapeBox : public UBaseShape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName Height;
	UPROPERTY(EditAnywhere) FName Width;
};

UCLASS()
class UShapeSquare : public UBaseShape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName Radius;
};

USTRUCT()
struct FShapeContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) UBaseShape* ShapeAlpha;
	UPROPERTY(EditAnywhere) UBaseShape* ShapeBeta;
	UPROPERTY(EditAnywhere) UBaseShape* ShapeGamma;
};

UENUM()
enum class EFootbar : int64
{
	Foo, Bar, Bart
};

UENUM()
enum class EIntBased : int32
{
	Zero = 0,
	NegOne = -1,
	PosOne = 1,
};

UENUM(BlueprintType, meta = (BitFlags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBPFlags : uint8
{
	None = 0,
	Alpha = 0x1 << 0,
	Beta = 0x1 << 1,
	Gamma = 0x1 << 2,
};
ENUM_CLASS_FLAGS(EBPFlags);

USTRUCT()
struct FStructWithEnum
{
	GENERATED_BODY()

	UPROPERTY() EFootbar Enum1;
	UPROPERTY() EIntBased Enum2;
	UPROPERTY() EBPFlags Enum3;
};


USTRUCT()
struct FStructWithText
{
	GENERATED_BODY()

	UPROPERTY() FText Text1;
};

USTRUCT()
struct FStructWithClassReference
{
	GENERATED_BODY()

	UPROPERTY() UClass* Cls1;
	UPROPERTY() UClass* Cls2;
	UPROPERTY() TSubclassOf<UBaseShape> Cls3;
};

USTRUCT()
struct FStructWithScriptStruct
{
	GENERATED_BODY()

	UPROPERTY() UScriptStruct* Struct1;
};

USTRUCT()
struct FStructWithSoftObjectPtr
{
	GENERATED_BODY()

	UPROPERTY() TWeakObjectPtr<UObject> Weak1;
	UPROPERTY() TLazyObjectPtr<UObject> Lazy1;
	UPROPERTY() TSoftObjectPtr<UObject> Soft1;
	UPROPERTY() TSoftClassPtr<UBaseShape> Soft2;

	//	UHT doesn't recognize this
	//UPROPERTY() FSoftObjectPtr Soft2;
};


//	Interfaces, never really got into these
//	!!! need `GENERATED_BODY` or elsewise it would NOT work
UINTERFACE()
class UFooInterface : public UInterface
{ 
	GENERATED_BODY()
};

class IFooInterface
{
	GENERATED_BODY()
	
public:

	virtual FString GetFooName();

	FString FooStr;
};


UCLASS()
class UInterfacedAlpha : public UObject, public IFooInterface
{
public:
	GENERATED_BODY()

	UInterfacedAlpha();
};


UCLASS()
class UInterfacedBeta : public UObject, public IFooInterface
{
public:
	GENERATED_BODY()

	UInterfacedBeta();
};


USTRUCT()
struct FStructWithInterface
{
	GENERATED_BODY()

	UPROPERTY() TScriptInterface<IFooInterface> FooInterface;
	UPROPERTY() TScriptInterface<IFooInterface> BarInterface;
};


DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(int, FIdentityDelegate, int, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynMulticastCallback, int, Value);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FDynMulticastSparseCallback, UDelegateClass, SparseCallback1, int, Value);

UCLASS()
class UDelegateClass : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION()
	int ReturnOne(int Int);

	UFUNCTION()
	void ReturnNone(int Int);

	UPROPERTY()
	FDynMulticastSparseCallback SparseCallback1;
};


USTRUCT()
struct FStructWithDelegate
{
	GENERATED_BODY()

	UPROPERTY() FIdentityDelegate Delegate1;
	UPROPERTY() FDynMulticastCallback DelgateGroup2;
};



