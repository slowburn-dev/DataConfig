#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FWriter
{
	FWriter();
	virtual ~FWriter();

	virtual FResult Peek(EDataEntry Next);

	virtual FResult WriteBool(bool Value);
	virtual FResult WriteName(const FName& Value);
	virtual FResult WriteString(const FString& Value);

	virtual FResult WriteStructRoot(const FName& Name);
	virtual FResult WriteStructEnd(const FName& Name);

	virtual FResult WriteClassRoot(const FClassPropertyStat& Class);
	virtual FResult WriteClassEnd(const FClassPropertyStat& Class);

	virtual FResult WriteMapRoot();
	virtual FResult WriteMapEnd();

	virtual FResult WriteArrayRoot();
	virtual FResult WriteArrayEnd();

};


}	// namespace DataConfig

