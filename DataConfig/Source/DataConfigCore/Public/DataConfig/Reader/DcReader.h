#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcReader
{
	virtual ~FDcReader();

	virtual EDcDataEntry Peek();

	virtual FDcResult ReadNil();

	virtual FDcResult ReadBool(bool* OutPtr);
	virtual FDcResult ReadName(FName* OutPtr);
	virtual FDcResult ReadString(FString* OutPtr);

	virtual FDcResult ReadStructRoot(FName* OutNamePtr);
	virtual FDcResult ReadStructEnd(FName* OutNamePtr);

	virtual FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr);
	virtual FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr);

	virtual FDcResult ReadMapRoot();
	virtual FDcResult ReadMapEnd();

	virtual FDcResult ReadArrayRoot();
	virtual FDcResult ReadArrayEnd();

	virtual FDcResult ReadReference(UObject** OutPtr);

};

