#include <Reader/PropertyReader.h>
#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Templates/Casts.h"
#include "Misc/TVariant.h"

struct Unknown {
	//	pass
};

struct Nil {
	//	pass
};

struct StateClassRoot
{
	TWeakObjectPtr<UObject> ClassObject;

	StateClassRoot(UObject* ClassObject)
		: ClassObject(ClassObject)
	{}
};

struct StateClassProperty
{
	TWeakObjectPtr<UObject> ClassObject;
	TWeakObjectPtr<UProperty> Property;

	StateClassProperty(UObject* ClassObject, UProperty* Property)
		: ClassObject(ClassObject)
		, Property(Property)
	{}
};

struct StateStructRoot 
{
	void* StructPtr;
	TWeakObjectPtr<UScriptStruct> StructClass;

	StateStructRoot(void* StructPtr, UScriptStruct* StructClass)
		: StructPtr(StructPtr)
		, StructClass(StructClass)
	{}
};

struct StateStructProperty 
{
	void* StructPtr;
	TWeakObjectPtr<UScriptStruct> StructClass;
	TWeakObjectPtr<UProperty> Property;

	StateStructProperty(void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
		: StructPtr(StructPtr)
		, StructClass(StructClass)
		, Property(Property)
	{}
};

struct StatePrimitive 
{
	void* PrimitivePtr;
	TWeakObjectPtr<UProperty> Property;

	StatePrimitive(void* PrimitivePtr, UProperty* Property)
		: PrimitivePtr(PrimitivePtr)
		, Property(Property)
	{}
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

FORCEINLINE ReaderState& GetState(FPropertyReader* Self)
{
	return *((ReaderState*)(&Self->ImplStorage));
}

FPropertyReader::FPropertyReader()
{
	static_assert(sizeof(ReaderState) <= sizeof(ImplStorageType), "impl storage size too small");
	new(&ImplStorage) ReaderState(TInPlaceType<Unknown>{});
}

FPropertyReader::FPropertyReader(UObject* ClassObject)
	: FPropertyReader()
{
	GetState(this).Emplace<StateClassRoot>(ClassObject);
}

FPropertyReader::FPropertyReader(UObject* ClassObject, UProperty* Property)
	: FPropertyReader()
{
	GetState(this).Emplace<StateClassProperty>(ClassObject, Property);
}

FPropertyReader::FPropertyReader(void* StructPtr, UScriptStruct* StructClass)
	: FPropertyReader()
{
	GetState(this).Emplace<StateStructRoot>(StructPtr, StructClass);
}

FPropertyReader::FPropertyReader(void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
	: FPropertyReader()
{
	GetState(this).Emplace<StateStructProperty>(StructPtr, StructClass, Property);
}

FPropertyReader::FPropertyReader(void* PrimitivePtr, UProperty* Property)
	: FPropertyReader()
{
	GetState(this).Emplace<StatePrimitive>(PrimitivePtr, Property);
}

FPropertyReader::~FPropertyReader()
{
	GetState(this).~ReaderState();
}

FVisitResult FPropertyReader::ReadBool(FVisitor &Visitor)
{
	if (StatePrimitive* StatePtr = GetState(this).TryGet<StatePrimitive>())
	{
		if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(StatePtr->Property))
		{
			bool Value = BoolProperty->GetPropertyValue(StatePtr->PrimitivePtr);
			return Visitor.VisitBool(Value);
		}
		else
		{
			return FVisitResult::Fail(TEXT("failed to get BoolProperty"));
		}
	}
	else
	{
		return FVisitResult::Fail(TEXT("expect bool but in non primitive state"));
	}
}

