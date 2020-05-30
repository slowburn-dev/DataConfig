#pragma once

#include "CoreMinimal.h"
#include "DataConfigTypes.h"

namespace DataConfig
{

struct FContextStorage;

struct DATACONFIGCORE_API FReader
{
	virtual ~FReader();

	virtual EDataEntry Peek();

	virtual FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr);
	virtual FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr);
	virtual FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr);

	virtual FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr);
	virtual FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr);

	virtual FResult ReadMapRoot(FContextStorage* CtxPtr);
	virtual FResult ReadMapEnd(FContextStorage* CtxPtr);

};



} // namespace DataConfig



