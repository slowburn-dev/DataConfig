#pragma once

#include "CoreMinimal.h"

class UField;

struct DATACONFIGCORE_API FDcPropertyDatum
{
	FFieldVariant Property;
	void* DataPtr;

	FDcPropertyDatum();
	FDcPropertyDatum(FField* InProperty, void* InDataPtr);

	FDcPropertyDatum(UObject* ClassObject);
	FDcPropertyDatum(UClass* Class, UObject* ClassObject);
	FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr);

	FORCEINLINE bool IsNone() const
	{
		check((!Property.IsValid() && DataPtr == nullptr) || (Property.IsValid() && DataPtr != nullptr));
		return Property == nullptr;
	}

	template<typename TProperty = FField>
	FORCEINLINE TProperty* CastFieldChecked() const
	{
		static_assert(TIsDerivedFrom<TProperty, FField>::Value, "expect TProperty to be a field");
		return ::CastFieldChecked<TProperty>(Property.ToField());
	}

	template<typename TProperty = FField>
	FORCEINLINE TProperty* CastField() const
	{
		static_assert(TIsDerivedFrom<TProperty, FField>::Value, "expect TProperty to be a field");
		return ::CastField<TProperty>(Property.ToField());
	}

	FORCEINLINE UScriptStruct* CastUScriptStructChecked() const
	{
		check(Property.IsUObject());
		return ::CastChecked<UScriptStruct>(Property.ToUObject());
	}

	FORCEINLINE UClass* CastUClassChecked() const
	{
		check(Property.IsUObject());
		return ::CastChecked<UClass>(Property.ToUObject());
	}

	static const FDcPropertyDatum NONE;
};



