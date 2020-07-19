#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

namespace DataConfig
{

struct FContextStorage;

struct DATACONFIGCORE_API FReader
{
	virtual ~FReader();

	virtual EDataEntry Peek();

	virtual FResult ReadNil(FContextStorage* CtxPtr);

	virtual FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr);
	virtual FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr);
	virtual FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr);

	virtual FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr);
	virtual FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr);

	virtual FResult ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr);
	virtual FResult ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr);

	virtual FResult ReadMapRoot(FContextStorage* CtxPtr);
	virtual FResult ReadMapEnd(FContextStorage* CtxPtr);

	virtual FResult ReadArrayRoot(FContextStorage* CtxPtr);
	virtual FResult ReadArrayEnd(FContextStorage* CtxPtr);

	virtual FResult ReadReference(UObject** OutPtr, FContextStorage* CtxPtr);

};



} // namespace DataConfig



