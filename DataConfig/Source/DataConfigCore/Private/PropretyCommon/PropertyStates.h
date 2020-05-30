#pragma once

#include "UObject/UnrealType.h"
//	TODO get rid of this TVariant by using tagged union for max compatibility
#include "Misc/TVariant.h"

namespace DataConfig
{

struct StateNil {
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

//	TODO we maybe only need two pointers, figure this out
//		 get rid of this when doing removing Variant
struct StateStructProperty 
{
	void* StructPtr;
	UScriptStruct* StructClass;	// TODO actually can get rid of this, it's on stack parent
	UProperty* Property;	// this is actually the Link, it's the iterator

	enum class EState 
	{
		ExpectKey,
		ExpectValue,
		Ended,
	};
	EState State;

	StateStructProperty(void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
		: StructPtr(StructPtr)
		, StructClass(StructClass)
		, Property(Property)
	{
		check(StructPtr != nullptr);
		check(IsValid(StructClass));
		//	relaxed as Writer starts with no property
		//check(IsValid(Property));
		State = EState::ExpectKey;
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

struct StateMappingRoot
{
	void* MapPtr;
	UMapProperty* MapProperty;

	StateMappingRoot(void* InMapPtr, UMapProperty* InMapProperty)
	{
		check(InMapPtr);
		check(IsValid(InMapProperty));
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
	}
};

struct StateMappingProperty
{
	FScriptMapHelper MapHelper;
	int Index;

	enum class EState 
	{
		ExpectKey,
		ExpectValue,
		Ended,
	};
	EState State;

	StateMappingProperty(void* InMapPtr, UMapProperty* InProperty)
		: MapHelper(InProperty, InMapPtr)
	{
		check(IsValid(InProperty));
		check(InMapPtr);
		Index = 0;
		State = EState::ExpectKey;
	}

};

using ReaderState = TVariant<
	StateNil,
	StateClassRoot,
	StateClassProperty,
	StateStructRoot,
	StateStructProperty,
	StateMappingRoot,
	StateMappingProperty,
	StatePrimitive
>;

using WriterState = TVariant<
	StateNil,
	StateClassRoot,
	StateClassProperty,
	StateStructRoot,
	StateStructProperty,
	StatePrimitive
>;


} // namespace DataConfig
