#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

//	Reads `/Path/To/BP` or `ClassName`
//FResult DATACONFIGCORE_API ClassReferenceDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

FResult DATACONFIGCORE_API ClassRootDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

FResult DATACONFIGCORE_API ObjectReferenceDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

FResult DATACONFIGCORE_API InstancedSubObjectDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);




} // namespace DataConfig






