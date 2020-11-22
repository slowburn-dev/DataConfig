#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

//	TODO merge w/ DcStartup/Shutdown into a single header?
void DATACONFIGCORE_API DcSetupDefaultDeserializeHandlers(FDcDeserializer& Deserializer);

