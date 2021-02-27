#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

struct FDcReader;
struct FDcPropertyWriter;
struct FDcDeserializer;

struct DATACONFIGCORE_API FDcDeserializeContext
{
	enum class EState
	{
		Uninitialized,
		Ready,
		DeserializeInProgress,
		DeserializeEnded,
	};

	EState State = EState::Uninitialized;

	FString Name;
	TArray<UObject*, TInlineAllocator<4>> Objects;
	TArray<FFieldVariant, TInlineAllocator<8>> Properties;

	FDcDeserializer* Deserializer;
	FDcReader* Reader;
	FDcPropertyWriter* Writer;

	FORCEINLINE FFieldVariant& TopProperty() 
	{
		checkf(Properties.Num(), TEXT("Expect TopProperty found none."));
		return Properties.Top();
	}

	FORCEINLINE UObject* TopObject() 
	{
		checkf(Objects.Num(), TEXT("Expect TopObject found none."));
		return Objects.Top();
	}

	FDcResult Prepare();
};

using FDcDeserializeDelegateSignature = FDcResult(*)(FDcDeserializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(FDcResult, FDcDeserializeDelegate, FDcDeserializeContext&);

enum class EDcDeserializePredicateResult
{
	Pass,
	Process,
};

using FDcDeserializePredicateSignature = EDcDeserializePredicateResult(*)(FDcDeserializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(EDcDeserializePredicateResult, FDcDeserializePredicate, FDcDeserializeContext&);

struct DATACONFIGCORE_API FDcScopedProperty
{
	FDcScopedProperty(FDcDeserializeContext& InCtx);
	~FDcScopedProperty();

	FDcResult PushProperty();

	FFieldVariant Property;
	FDcDeserializeContext& Ctx;
};

struct DATACONFIGCORE_API FDcScopedObject
{
	FDcScopedObject(FDcDeserializeContext& InCtx, UObject* InObject);
	~FDcScopedObject();

	UObject* Object;
	FDcDeserializeContext& Ctx;
};





