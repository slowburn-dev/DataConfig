#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

struct FDcReader;
struct FDcPropertyWriter;
struct FDcDeserializer;

struct DATACONFIGCORE_API FDcDeserializeContext
{
	FString Name;
	TArray<FName, TInlineAllocator<4>> Pathes;
	TArray<UObject*, TInlineAllocator<4>> Objects;
	TArray<UField*, TInlineAllocator<8>> Properties;

	FDcDeserializer* Deserializer;
	FDcReader* Reader;
	FDcPropertyWriter* Writer;

	FORCEINLINE UField* TopProperty() { return Properties.Top(); }
	FORCEINLINE UObject* TopObject() { return Objects.Top(); }
};

enum class EDcDeserializeResult
{
	Unknown,
	CanNotProcess,
	Processed,
	//	failed is implicit as it returns fail
};

FORCEINLINE FDcResult DcOkWithCanNotProcess(EDcDeserializeResult& OutResult)
{
	OutResult = EDcDeserializeResult::CanNotProcess;
	return DcOk();
}

FORCEINLINE FDcResult DcOkWithProcessed(EDcDeserializeResult& OutResult)
{
	OutResult = EDcDeserializeResult::Processed;
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
	FDcScopedProperty(FDcDeserializeContext& InCtx)
		: Property(nullptr)
		, Ctx(InCtx)
	{}

	FDcResult PushProperty();
	~FDcScopedProperty();

	UField* Property;
	FDcDeserializeContext& Ctx;
};

struct DATACONFIGCORE_API FDcScopedObject
{
	FDcScopedObject(FDcDeserializeContext& InCtx, UObject* InObject);
	~FDcScopedObject();

	UObject* Object;
	FDcDeserializeContext& Ctx;
};





