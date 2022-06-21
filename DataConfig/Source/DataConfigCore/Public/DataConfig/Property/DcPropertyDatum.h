#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "UObject/UnrealType.h"

struct DATACONFIGCORE_API FDcPropertyDatum
{
	FFieldVariant Property;
	void* DataPtr;

	FDcPropertyDatum();
	FDcPropertyDatum(FField* InProperty, void* InDataPtr);

	FDcPropertyDatum(UClass* Class, UObject* ClassObject);
	FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr);

	explicit FDcPropertyDatum(UObject* ClassObject);

	template<
		typename T,
		typename X = typename TEnableIf<DcTypeUtils::TIsUStruct<T>::Value, void>::Type
	>
	explicit FDcPropertyDatum(
		T* InStructPtr
	) : FDcPropertyDatum(TBaseStructure<T>::Get(), InStructPtr)
	{}

	FORCEINLINE bool IsNone() const
	{
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

	FORCEINLINE UScriptStruct* CastUScriptStruct() const
	{
		return ::Cast<UScriptStruct>(Property.ToUObject());
	}

	FORCEINLINE UClass* CastUClassChecked() const
	{
		check(Property.IsUObject());
		return ::CastChecked<UClass>(Property.ToUObject());
	}

	FORCEINLINE UClass* CastUClass() const
	{
		return ::Cast<UClass>(Property.ToUObject());
	}

	FORCEINLINE void Reset()
	{
		Property = nullptr;
		DataPtr = nullptr;
	}

	template<typename T>
	FORCEINLINE bool IsA() const { return Property.IsA<T>(); }


	static const FDcPropertyDatum NONE;
};

FORCEINLINE FDcPropertyDatum::FDcPropertyDatum()
{
	Property = FFieldVariant();
	DataPtr = nullptr;
}

FORCEINLINE FDcPropertyDatum::FDcPropertyDatum(FField* InProperty, void* InDataPtr)
{
	Property = InProperty;
	DataPtr = InDataPtr;
}

FORCEINLINE FDcPropertyDatum::FDcPropertyDatum(UObject* ClassObject)
{
	check(ClassObject);
	Property = ClassObject->GetClass();
	DataPtr = ClassObject;
}

FORCEINLINE FDcPropertyDatum::FDcPropertyDatum(UClass* Class, UObject* ClassObject)
{
	check(Class && ClassObject);
	Property = Class;
	DataPtr = ClassObject;
}

FORCEINLINE  FDcPropertyDatum::FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr)
{
	check(StructClass);
	Property = StructClass;
	DataPtr = StructPtr;
}
