#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

namespace DataConfig
{

struct FReader;
struct FPropertyWriter;
struct FDeserializer;

struct DATACONFIGCORE_API FDeserializeContext
{
	FString Name;
	TArray<FName, TInlineAllocator<4>> Pathes;
	TArray<UObject*, TInlineAllocator<4>> Objects;
	TArray<UField*, TInlineAllocator<8>> Properties;

	FDeserializer* Deserializer;
	FReader* Reader;
	FPropertyWriter* Writer;

	FORCEINLINE UField* TopProperty() { return Properties.Top(); }
	FORCEINLINE UObject* TopObject() { return Objects.Top(); }
};

enum class EDeserializeResult
{
	Unknown,
	CanNotProcess,
	Processed,
	//	failed is implicit as it returns fail
};

FORCEINLINE FResult OkWithCanNotProcess(EDeserializeResult& OutResult)
{
	OutResult = EDeserializeResult::CanNotProcess;
	return Ok();
}

FORCEINLINE FResult OkWithProcessed(EDeserializeResult& OutResult)
{
	OutResult = EDeserializeResult::Processed;
	return Ok();
}


using FDeserializeDelegateSignature = FResult(*)(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
DECLARE_DELEGATE_RetVal_TwoParams(FResult, FDeserializeDelegate, FDeserializeContext&, EDeserializeResult&);

enum class EDeserializePredicateResult
{
	Pass,
	Process,
};

using FDeserializePredicateSignature = EDeserializePredicateResult(*)(FDeserializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(EDeserializePredicateResult, FDeserializePredicate, FDeserializeContext&);

struct DATACONFIGCORE_API FScopedProperty
{
	FScopedProperty(FDeserializeContext& InCtx)
		: Property(nullptr)
		, Ctx(InCtx)
	{}

	FResult PushProperty();
	~FScopedProperty();

	UField* Property;
	FDeserializeContext& Ctx;
};

struct DATACONFIGCORE_API FScopedObject
{
	FScopedObject(FDeserializeContext& InCtx, UObject* InObject);
	~FScopedObject();

	UObject* Object;
	FDeserializeContext& Ctx;
};


} // namespace DataConfig




