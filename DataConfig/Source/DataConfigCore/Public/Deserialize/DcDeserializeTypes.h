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

	UObject* Obj = nullptr;

	FDeserializer* Deserializer;

	TArray<FName> Pathes;
};

struct DATACONFIGCORE_API IDeserializeConverter : public TSharedFromThis<IDeserializeConverter>
{
	virtual ~IDeserializeConverter();

	//	rename this to `Prepare` to indicate that this can have side effects
	//	guarentee that when Prepare returns true, Deserialize will use the same Arguments for processing
	virtual bool Prepare(FReader& Reader, FPropertyWriter& Writer, FPropertyDatum Datum, FDeserializeContext& Ctx) = 0;

	virtual FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FPropertyDatum Datum, FDeserializeContext& Ctx) = 0;
};


} // namespace DataConfig




