#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

struct FDcWriter;
struct FDcPropertyReader;
struct FDcSerializer;

struct DATACONFIGCORE_API FDcSerializeContext
{
	enum class EState
	{
		Uninitialized,
		Ready,
		SerializeInProgress,
		SerializeEnded,
	};

	EState State = EState::Uninitialized;

	TArray<FFieldVariant, TInlineAllocator<8>> Properties;

	FDcSerializer* Serializer = nullptr;
	FDcPropertyReader* Reader = nullptr;
	FDcWriter* Writer = nullptr;

	void* UserData = nullptr;

	FORCEINLINE FFieldVariant& TopProperty() 
	{
		checkf(Properties.Num(), TEXT("Expect TopProperty found none."));
		return Properties.Top();
	}

	FDcResult Prepare();
};

using FDcSerializeDelegateSignature = FDcResult(*)(FDcSerializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(FDcResult, FDcSerializeDelegate, FDcSerializeContext&);

enum class EDcSerializePredicateResult
{
	Pass,
	Process,
};

using FDcSerializePredicateSignature = EDcSerializePredicateResult(*)(FDcSerializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(EDcSerializePredicateResult, FDcSerializePredicate, FDcSerializeContext&);



