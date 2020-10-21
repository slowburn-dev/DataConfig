#pragma once

#include "CoreMinimal.h"

class UField;

namespace DataConfig
{

struct DATACONFIGCORE_API FDcPropertyDatum
{
	UField* Property;
	void* DataPtr;

	FDcPropertyDatum();
	FDcPropertyDatum(UField* InProperty, void* InDataPtr);

	FDcPropertyDatum(UObject* ClassObject);
	FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr);

	FORCEINLINE bool IsNone()
	{
		check((Property == nullptr && DataPtr == nullptr) || (Property != nullptr && DataPtr != nullptr));
		return Property == nullptr;
	}

	template<typename TProperty>
	FORCEINLINE TProperty* CastChecked()
	{
		return ::CastChecked<TProperty>(Property);
	}

	template<typename TProperty>
	FORCEINLINE TProperty* Cast()
	{
		return ::Cast<TProperty>(Property);
	}

	template<typename TProperty>
	FORCEINLINE bool IsA()
	{
		check(Property);
		return Property->IsA<TProperty>();
	}

	static const FDcPropertyDatum NONE;
};


}	// namespace DataConfig



