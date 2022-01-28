#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

enum class EDcJsonSerializeType
{
	Default,
	StringSoftLazy,	// Serialize Soft/Lazy references as string
};

DATACONFIGCORE_API void DcSetupJsonSerializeHandlers(FDcSerializer& Serializer, EDcJsonSerializeType Type = EDcJsonSerializeType::Default);

DATACONFIGCORE_API void DcSetupPropertyPipeSerializeHandlers(FDcSerializer& Serializer);

enum class EDcMsgPackSerializeType
{
	Default,
	StringSoftLazy,	// Serialize Soft/Lazy references as string
	InMemory,		// Serialize pointer/enum/FName etc as underlying integer values
};

DATACONFIGCORE_API void DcSetupMsgPackSerializeHandlers(FDcSerializer& Serializer, EDcMsgPackSerializeType Type = EDcMsgPackSerializeType::Default);

