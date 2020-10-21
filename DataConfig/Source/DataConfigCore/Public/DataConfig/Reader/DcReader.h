#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct FContextStorage;

struct DATACONFIGCORE_API FDcReader
{
	virtual ~FDcReader();

	virtual EDataEntry Peek();

	virtual FDcResult ReadNil(FContextStorage* CtxPtr);

	virtual FDcResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr);
	virtual FDcResult ReadName(FName* OutPtr, FContextStorage* CtxPtr);
	virtual FDcResult ReadString(FString* OutPtr, FContextStorage* CtxPtr);

	virtual FDcResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr);
	virtual FDcResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr);

	virtual FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr);
	virtual FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr);

	virtual FDcResult ReadMapRoot(FContextStorage* CtxPtr);
	virtual FDcResult ReadMapEnd(FContextStorage* CtxPtr);

	virtual FDcResult ReadArrayRoot(FContextStorage* CtxPtr);
	virtual FDcResult ReadArrayEnd(FContextStorage* CtxPtr);

	virtual FDcResult ReadReference(UObject** OutPtr, FContextStorage* CtxPtr);

};

