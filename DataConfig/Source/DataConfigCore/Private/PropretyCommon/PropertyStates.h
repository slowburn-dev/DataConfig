#pragma once

#include "UObject/UnrealType.h"
#include "Misc/TVariant.h"

namespace DataConfig
{

struct Unknown {
	//	pass
};

struct Nil {
	//	pass
};

struct StateClassRoot
{
	UObject* ClassObject;

	StateClassRoot(UObject* ClassObject)
		: ClassObject(ClassObject)
	{
		check(IsValid(ClassObject));
	}
};

struct StateClassProperty
{
	UObject* ClassObject;
	UProperty* Property;

	StateClassProperty(UObject* ClassObject, UProperty* Property)
		: ClassObject(ClassObject)
		, Property(Property)
	{
		check(IsValid(ClassObject));
		check(IsValid(Property));
	}
};

struct StateStructRoot 
{
	void* StructPtr;
	UScriptStruct* StructClass;

	StateStructRoot(void* StructPtr, UScriptStruct* StructClass)
		: StructPtr(StructPtr)
		, StructClass(StructClass)
	{
		check(StructPtr != nullptr);
		check(IsValid(StructClass));
	}
};

struct StateStructProperty 
{
	void* StructPtr;
	UScriptStruct* StructClass;
	UProperty* Property;

	StateStructProperty(void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
		: StructPtr(StructPtr)
		, StructClass(StructClass)
		, Property(Property)
	{
		check(StructPtr != nullptr);
		check(IsValid(StructClass));
		check(IsValid(Property));
	}
};

struct StatePrimitive 
{
	void* PrimitivePtr;
	UProperty* Property;

	StatePrimitive(void* PrimitivePtr, UProperty* Property)
		: PrimitivePtr(PrimitivePtr)
		, Property(Property)
	{
		check(PrimitivePtr != nullptr);
		check(IsValid(Property));
	}
};

using ReaderState = TVariant<
	Unknown,
	Nil,
	StateClassRoot,
	StateClassProperty,
	StateStructRoot,
	StateStructProperty,
	StatePrimitive
>;


} // namespace DataConfig
