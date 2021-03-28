#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

DATACONFIGCORE_API void DcSetupJsonDeserializeHandlers(FDcDeserializer& Deserializer);

DATACONFIGCORE_API void DcSetupPropertyPipeDeserializeHandlers(FDcDeserializer& Deserializer);

