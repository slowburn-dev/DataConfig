#pragma once
#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

namespace DcMsgPackHandlers
{

DATACONFIGCORE_API FDcResult HandlerTransientNameSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientTextSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientObjectSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientClassSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientSoftObjectSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientSoftClassSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientWeakObjectSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientLazyObjectSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientInterfaceSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientDelegateSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientMulticastInlineDelegateSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientMulticastSparseDelegateSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerTransientFieldPathSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientEnumSerialize(FDcSerializeContext& Ctx);

	
} // namespace DcMsgPackHandlers




