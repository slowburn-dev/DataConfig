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
	TArray<FName, TInlineAllocator<4>> Pathes;
	TArray<UObject*, TInlineAllocator<4>> Objects;
	TArray<FFieldVariant, TInlineAllocator<8>> Properties;

	FDcDeserializer* Deserializer;
	FDcReader* Reader;
	FDcPropertyWriter* Writer;

	FORCEINLINE FFieldVariant& TopProperty() { return Properties.Top(); }
	FORCEINLINE UObject* TopObject() { return Objects.Top(); }

	void Prepare();
};

enum class EDcDeserializeResult
{
	Unknown,
	FallThrough,		//	Pass and let other handlers to try 
	Success,			//	Success and finish processing
	//	note that failure is propagated through DC_FAIL just like other APIs
};

FORCEINLINE FDcResult DcOkWithFallThrough(EDcDeserializeResult& OutResult)
{
	OutResult = EDcDeserializeResult::FallThrough;
	return DcOk();
}

FORCEINLINE FDcResult DcOkWithProcessed(EDcDeserializeResult& OutResult)
{
	OutResult = EDcDeserializeResult::Success;
	return DcOk();
}


using FDcDeserializeDelegateSignature = FDcResult(*)(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DECLARE_DELEGATE_RetVal_TwoParams(FDcResult, FDcDeserializeDelegate, FDcDeserializeContext&, EDcDeserializeResult&);

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





