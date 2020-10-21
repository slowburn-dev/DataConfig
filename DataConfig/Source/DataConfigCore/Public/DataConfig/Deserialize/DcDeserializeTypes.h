#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

namespace DataConfig
{

struct FDcReader;
struct FDcPropertyWriter;
struct FDeserializer;

struct DATACONFIGCORE_API FDeserializeContext
{
	FString Name;
	TArray<FName, TInlineAllocator<4>> Pathes;
	TArray<UObject*, TInlineAllocator<4>> Objects;
	TArray<UField*, TInlineAllocator<8>> Properties;

	FDeserializer* Deserializer;
	FDcReader* Reader;
	FDcPropertyWriter* Writer;

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

FORCEINLINE FDcResult OkWithCanNotProcess(EDeserializeResult& OutResult)
{
	OutResult = EDeserializeResult::CanNotProcess;
	return DcOk();
}

FORCEINLINE FDcResult OkWithProcessed(EDeserializeResult& OutResult)
{
	OutResult = EDeserializeResult::Processed;
	return DcOk();
}


using FDeserializeDelegateSignature = FDcResult(*)(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
DECLARE_DELEGATE_RetVal_TwoParams(FDcResult, FDeserializeDelegate, FDeserializeContext&, EDeserializeResult&);

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

	FDcResult PushProperty();
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




