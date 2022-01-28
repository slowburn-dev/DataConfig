#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcMsgPackHandlers
{

DATACONFIGCORE_API FDcResult HandlerTransientNameDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientTextDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientClassDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientSoftObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientSoftClassDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientWeakObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientLazyObjectDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientInterfaceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientDelegateDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientMulticastInlineDelegateDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientMulticastSparseDelegateDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientFieldPathDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientEnumDeserialize(FDcDeserializeContext& Ctx);

} // namespace DcMsgPackHandlers


