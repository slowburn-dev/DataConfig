#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcWriter
{
	FDcWriter();
	virtual ~FDcWriter();

	virtual FDcResult Peek(EDataEntry Next);

	virtual FDcResult WriteNil();
	virtual FDcResult WriteBool(bool Value);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteString(const FString& Value);

	virtual FDcResult WriteStructRoot(const FName& Name);
	virtual FDcResult WriteStructEnd(const FName& Name);

	virtual FDcResult WriteClassRoot(const FDcClassPropertyStat& Class);
	virtual FDcResult WriteClassEnd(const FDcClassPropertyStat& Class);

	virtual FDcResult WriteMapRoot();
	virtual FDcResult WriteMapEnd();

	virtual FDcResult WriteArrayRoot();
	virtual FDcResult WriteArrayEnd();

	virtual FDcResult WriteReference(UObject* Value);
};


