#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Property/DcPropertyDatum.h"

namespace DataConfig
{

struct FReader;
struct FPropertyWriter;
struct FDeserializer;

struct DATACONFIGCORE_API FDeserializeContext
{
	FString Name;
	TArray<FName> Pathes;
	TArray<UField*> Properties;

	FDeserializer* Deserializer;
	FReader* Reader;
	FPropertyWriter* Writer;

	FORCEINLINE UField* TopProperty()
	{
		return Properties.Top();
	}
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


struct FScopedProperty
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



struct DATACONFIGCORE_API IDeserializeConverter : public TSharedFromThis<IDeserializeConverter>
{
	virtual ~IDeserializeConverter();

	//	rename this to `Prepare` to indicate that this can have side effects
	//	guarentee that when Prepare returns true, Deserialize will use the same Arguments for processing
	virtual bool Prepare(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx) = 0;

	virtual FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx) = 0;
};


} // namespace DataConfig




