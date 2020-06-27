#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDeserializer : public FNoncopyable
{
	FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx);

	//	note that this must be const ref so it can be bound to rvalue
	void AddConverter(const TSharedRef<IDeserializeConverter>& NewConverter);

	TArray<TSharedRef<IDeserializeConverter>> Converters;
};


} // namespace DataConfig





