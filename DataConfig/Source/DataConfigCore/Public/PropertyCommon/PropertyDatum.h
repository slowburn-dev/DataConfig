#pragma once

#include "CoreMinimal.h"

class UField;

namespace DataConfig
{

struct DATACONFIGCORE_API FPropertyDatum
{
	UField* Property;
	void* DataPtr;

	FPropertyDatum();
	FPropertyDatum(UField* InProperty, void* InDataPtr);

	FPropertyDatum(UObject* ClassObject);
	FPropertyDatum(UScriptStruct* StructClass, void* StructPtr);

	FORCEINLINE bool IsNone()
	{
		check((Property == nullptr && DataPtr == nullptr) || (Property != nullptr && DataPtr != nullptr));
		return Property == nullptr;
	}

	static const FPropertyDatum NONE;
};


}	// namespace DataConfig



