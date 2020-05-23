#pragma once

#include "CoreMinimal.h"
#include "DataConfigTypes.h"

namespace DataConfig
{

struct FWriterStorage;

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
};


}	// namespace DataConfig


