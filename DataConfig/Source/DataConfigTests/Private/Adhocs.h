#pragma once

#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
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

void PropertyVisitorRoundtrip();
void JsonReader1();
void UEStructSerializer();
void DeserializeSimple();




