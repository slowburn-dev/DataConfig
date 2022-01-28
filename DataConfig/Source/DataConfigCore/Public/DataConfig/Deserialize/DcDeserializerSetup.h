#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

enum class EDcJsonDeserializeType
{
	Default,
	StringSoftLazy,	// Deserialize Soft/Lazy references as string
};

DATACONFIGCORE_API void DcSetupJsonDeserializeHandlers(FDcDeserializer& Deserializer, EDcJsonDeserializeType Type = EDcJsonDeserializeType::Default);

DATACONFIGCORE_API void DcSetupPropertyPipeDeserializeHandlers(FDcDeserializer& Deserializer);

enum class EDcMsgPackDeserializeType
{
	Default,
	StringSoftLazy,	// Deserialize Soft/Lazy references as string
	InMemory,		// Deserialize pointer/enum/FName etc as underlying integer values
};

DATACONFIGCORE_API void DcSetupMsgPackDeserializeHandlers(FDcDeserializer& Deserializer, EDcMsgPackDeserializeType Type = EDcMsgPackDeserializeType::Default);

