#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Templates/Casts.h"
#include "Misc/TVariant.h"
#include <Reader/PropertyReader.h>
#include <Reader/ReaderErrorCodes.h>

namespace DataConfig {

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

static FORCEINLINE ReaderState& GetState(FPropertyReader* Self)
{
	return *((ReaderState*)(&Self->ImplStorage));
}

template<typename TState>
static TState GetState(FPropertyReader* Self) {
	return GetState(Self).Get<TState>();
}

template<typename TState>
static TState* TryGetState(FPropertyReader* Self) {
	return GetState(Self).TryGet<TState>();
}

static bool IsEffectiveProperty(UProperty* Property)
{
	check(Property);
	return Property->IsA<UBoolProperty>()
		|| Property->IsA<UFloatProperty>()
		|| Property->IsA<UDoubleProperty>()
		|| Property->IsA<UIntProperty>()
		|| Property->IsA<UUInt32Property>()
		|| Property->IsA<UStrProperty>()
		|| Property->IsA<UNameProperty>()
		|| Property->IsA<UStructProperty>()
		|| Property->IsA<UObjectProperty>()
		|| Property->IsA<UMapProperty>()
		|| Property->IsA<UArrayProperty>();
}

static size_t CountEffectiveProperties(UStruct* Struct)
{
	check(Struct);
	size_t EffectiveCount = 0;
	for (UProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (IsEffectiveProperty(Property))
		{
			++EffectiveCount;
		}
	}

	return EffectiveCount;
}

static FVisitResult DispatchReadByProperty(FPropertyReader* Reader, FVisitor& Visitor, UProperty* Property)
{
	check(Reader);
	check(Property);
	if (Property->IsA<UBoolProperty>())
		return Reader->ReadBool(Visitor);
	else if (Property->IsA<UNameProperty>())
		return Reader->ReadName(Visitor);
	else if (Property->IsA<UStrProperty>())
		return Reader->ReadString(Visitor);

	return Fail(EReaderErrorCode::DispatchAnyFail);
}

template<typename TPrimitive, typename TProperty, EReaderErrorCode ErrCode>
FVisitResult TryGetPrimitive(FPropertyReader* Reader, TPrimitive& OutT)
{
	if (StateClassProperty* CPStatePtr = TryGetState<StateClassProperty>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(CPStatePtr->Property))
		{
			OutT = OutProperty->GetPropertyValue(
				OutProperty->ContainerPtrToValuePtr<TPrimitive>(CPStatePtr->ClassObject)
			);
			return Ok();
		}
	}
	else if (StateStructProperty* SPStatePtr = TryGetState<StateStructProperty>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(SPStatePtr->Property))
		{
			OutT = OutProperty->GetPropertyValue(
				OutProperty->ContainerPtrToValuePtr<TPrimitive>(SPStatePtr->StructPtr)
			);
			return Ok();
		}
	}
	else if (StatePrimitive* PStatePtr = TryGetState<StatePrimitive>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(PStatePtr->Property))
		{
			OutT = OutProperty->GetPropertyValue(PStatePtr->PrimitivePtr);
			return Ok();
		}
	}

	return Fail(ErrCode);
}

template<typename TPrimitive, typename TProperty, EReaderErrorCode ErrCode, typename TMethod>
FVisitResult ReadPrimitive(FPropertyReader* Reader, FVisitor& Visitor, TMethod Method)
{
	TPrimitive Value;
	FVisitResult Result = TryGetPrimitive<TPrimitive, TProperty, ErrCode>(Reader, Value);
	if (Result.Ok())
	{
		return (Visitor.*Method)(Value);
	}
	else
	{
		return Result;
	}
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

FVisitResult FPropertyReader::ReadAny(FVisitor &Visitor)
{
	//	this is basically a dispatch function
	if (StateClassRoot* CRStatePtr = TryGetState<StateClassRoot>(this))
	{
		//	TODO class root
	}
	else if (StateClassProperty* CPStatePtr = TryGetState<StateClassProperty>(this))
	{
		return DispatchReadByProperty(this, Visitor, CPStatePtr->Property);
	}
	else if (StateStructRoot* SRStatePtr = TryGetState<StateStructRoot>(this))
	{
		return ReadStruct(Visitor);
	}
	else if (StateStructProperty* SPStatePtr = TryGetState<StateStructProperty>(this))
	{
		return DispatchReadByProperty(this, Visitor, SPStatePtr->Property);
	}
	else if (StatePrimitive* PStatePtr = TryGetState<StatePrimitive>(this))
	{
		return DispatchReadByProperty(this, Visitor, PStatePtr->Property);
	}

	return Fail(EReaderErrorCode::DispatchAnyFail);
}

FVisitResult FPropertyReader::ReadBool(FVisitor &Visitor)
{
	return ReadPrimitive<bool, UBoolProperty, EReaderErrorCode::ExpectBoolFail>(this, Visitor, &FVisitor::VisitBool);
}

FVisitResult FPropertyReader::ReadName(FVisitor &Visitor)
{
	return ReadPrimitive<FName, UNameProperty, EReaderErrorCode::ExpectNameFail>(this, Visitor, &FVisitor::VisitName);
}

FVisitResult FPropertyReader::ReadString(FVisitor &Visitor)
{
	return ReadPrimitive<FString, UStrProperty, EReaderErrorCode::ExpectStringFail>(this, Visitor, &FVisitor::VisitString);
}

FVisitResult FPropertyReader::ReadStruct(FVisitor &Visitor)
{
	if (StateStructRoot* StatePtr = TryGetState<StateStructRoot>(this))
	{
		FStructMapAccess StructMapAccess(this);
		return Visitor.VisitMap(StructMapAccess);
	}
	else
	{
		return Fail(EReaderErrorCode::ExpectStructFail);
	}
}

FPropertyReader::FStructMapAccess::FStructMapAccess(FPropertyReader* ParentPtr)
	: Parent(*ParentPtr)
{
 	auto StructClass = GetState(&Parent).Get<StateStructRoot>().StructClass;
	Link = StructClass ? StructClass->PropertyLink : nullptr;
}

FVisitResult FPropertyReader::FStructMapAccess::Num(size_t& OutNum)
{
	//	TODO check struct validity
	OutNum = CountEffectiveProperties(GetState(&Parent).Get<StateStructRoot>().StructClass);
	return Ok();
}

FVisitResult FPropertyReader::FStructMapAccess::HasPending(bool& bOutHasPending)
{
	bOutHasPending = Link != nullptr;
	return Ok();
}

FVisitResult FPropertyReader::FStructMapAccess::ReadKey(FVisitor &Visitor)
{
	if (Link != nullptr)
	{
		return Visitor.VisitName(Link->GetFName());
	}
	else
	{
		return Fail(EReaderErrorCode::OutOfBoundMapKeyRead);
	}
}

FVisitResult FPropertyReader::FStructMapAccess::ReadValue(FVisitor &Visitor)
{
	if (Link != nullptr)
	{
		auto& StructRootState = GetState(&Parent).Get<StateStructRoot>();
		//	we're already constructing other readers...
		//	maybe we don't need the state?
		FPropertyReader ValueReader(
			StructRootState.StructPtr,
			StructRootState.StructClass,
			Link
		);
		return ValueReader.ReadAny(Visitor);
	}
	else
	{
		return Fail(EReaderErrorCode::OutOfBoundMapValueRead);
	}
}

FVisitResult FPropertyReader::FStructMapAccess::Next()
{
	if (Link != nullptr)
	{
		Link = Link->PropertyLinkNext;
	}

	return Ok();
}

} // namespace DataConfig
