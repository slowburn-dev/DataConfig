#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "Reader/PropertyReader.h"
#include "DataConfigErrorCodes.h"
#include "PropretyCommon/PropertyStates.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig {

static FORCEINLINE ReaderState& GetState(FPropertyReader* Self)
{
	return *((ReaderState*)(&Self->ImplStorage));
}

template<typename TState>
static TState& GetState(FPropertyReader* Self) {
	return GetState(Self).Get<TState>();
}

template<typename TState>
static TState* TryGetState(FPropertyReader* Self) {
	return GetState(Self).TryGet<TState>();
}

static FResult DispatchReadByProperty(FPropertyReader* Reader, FVisitor& Visitor, UProperty* Property)
{
	check(Reader);
	check(Property);
	if (Property->IsA<UBoolProperty>())
		return Reader->ReadBool(Visitor);
	else if (Property->IsA<UNameProperty>())
		return Reader->ReadName(Visitor);
	else if (Property->IsA<UStrProperty>())
		return Reader->ReadString(Visitor);

	return Fail(EErrorCode::DispatchAnyFail);
}

template<typename TPrimitive, typename TProperty, EErrorCode ErrCode>
FResult TryGetPrimitive(FPropertyReader* Reader, TPrimitive& OutT)
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

template<typename TPrimitive, typename TProperty, EErrorCode ErrCode, typename TMethod>
FResult ReadPrimitive(FPropertyReader* Reader, FVisitor& Visitor, TMethod Method)
{
	TPrimitive Value;
	FResult Result = TryGetPrimitive<TPrimitive, TProperty, ErrCode>(Reader, Value);
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
	static_assert(sizeof(FPropertyReader) <= 64, "larger than cacheline");
	static_assert(sizeof(ReaderState) <= sizeof(ImplStorageType), "impl storage size too small");
	new(&ImplStorage) ReaderState(TInPlaceType<StateUnknown>{});
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

FResult FPropertyReader::ReadAny(FVisitor &Visitor)
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

	return Fail(EErrorCode::DispatchAnyFail);
}

FResult FPropertyReader::ReadBool(FVisitor &Visitor) 
{
	return ReadPrimitive<bool, UBoolProperty, EErrorCode::ExpectBoolFail>(this, Visitor, &FVisitor::VisitBool);
}

FResult FPropertyReader::ReadName(FVisitor &Visitor)
{
	return ReadPrimitive<FName, UNameProperty, EErrorCode::ExpectNameFail>(this, Visitor, &FVisitor::VisitName);
}

FResult FPropertyReader::ReadString(FVisitor &Visitor)
{
	return ReadPrimitive<FString, UStrProperty, EErrorCode::ExpectStringFail>(this, Visitor, &FVisitor::VisitString);
}

FResult FPropertyReader::ReadFloat(FVisitor &Visitor)
{
	return ReadPrimitive<float, UFloatProperty, EErrorCode::ExpectFloatFail>(this, Visitor, &FVisitor::VisitFloat);
}

FResult FPropertyReader::ReadDouble(FVisitor &Visitor)
{
	return ReadPrimitive<double, UDoubleProperty, EErrorCode::ExpectDoubleFail>(this, Visitor, &FVisitor::VisitDouble);
}

FResult FPropertyReader::ReadInt8(FVisitor &Visitor)
{
	return ReadPrimitive<int8, UInt8Property, EErrorCode::ExpectInt8Fail>(this, Visitor, &FVisitor::VisitInt8);
}

FResult FPropertyReader::ReadInt16(FVisitor &Visitor)
{
	return ReadPrimitive<int16, UInt16Property, EErrorCode::ExpectInt16Fail>(this, Visitor, &FVisitor::VisitInt16);
}

FResult FPropertyReader::ReadInt(FVisitor &Visitor)
{
	return ReadPrimitive<int, UIntProperty, EErrorCode::ExpectIntFail>(this, Visitor, &FVisitor::VisitInt);
}

FResult FPropertyReader::ReadInt64(FVisitor &Visitor)
{
	return ReadPrimitive<int64, UInt64Property, EErrorCode::ExpectInt64Fail>(this, Visitor, &FVisitor::VisitInt64);
}

FResult FPropertyReader::ReadByte(FVisitor &Visitor)
{
	return ReadPrimitive<uint8, UByteProperty, EErrorCode::ExpectByteFail>(this, Visitor, &FVisitor::VisitByte);
}

FResult FPropertyReader::ReadUInt16(FVisitor &Visitor)
{
	return ReadPrimitive<uint16, UInt16Property, EErrorCode::ExpectUInt16Fail>(this, Visitor, &FVisitor::VisitUInt16);
}

FResult FPropertyReader::ReadUInt32(FVisitor &Visitor)
{
	return ReadPrimitive<uint32, UUInt32Property, EErrorCode::ExpectUInt32Fail>(this, Visitor, &FVisitor::VisitUInt32);
}

FResult FPropertyReader::ReadUInt64(FVisitor &Visitor)
{
	return ReadPrimitive<uint64, UUInt64Property, EErrorCode::ExpectUInt64Fail>(this, Visitor, &FVisitor::VisitUInt64);
}

FResult FPropertyReader::ReadClass(FVisitor &Visitor)
{
	if (StateClassRoot* StatePtr = TryGetState<StateClassRoot>(this))
	{
		//	TODO
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ExpectClassFail);
	}
}

FResult FPropertyReader::ReadStruct(FVisitor &Visitor)
{
	if (StateStructRoot* StatePtr = TryGetState<StateStructRoot>(this))
	{
		check(StatePtr->StructClass);
		FStructMapAccess StructMapAccess(this);
		return Visitor.VisitStruct(StatePtr->StructClass->GetFName(), StructMapAccess);
	}
	else
	{
		return Fail(EErrorCode::ExpectStructFail);
	}
}

FResult FPropertyReader::ReadMap(FVisitor &Visitor)
{
	return Fail(EErrorCode::ExpectMapFail);
}

FPropertyReader::FStructMapAccess::FStructMapAccess(FPropertyReader* ParentPtr)
	: Parent(*ParentPtr)
{
 	auto StructClass = GetState(&Parent).Get<StateStructRoot>().StructClass;
	Link = FirstEffectiveProperty(StructClass->PropertyLink);
}

FResult FPropertyReader::FStructMapAccess::Num(TOptional<size_t>& OutNum)
{
	return Ok();
}

FResult FPropertyReader::FStructMapAccess::HasPending(bool& bOutHasPending)
{
	bOutHasPending = Link != nullptr;
	return Ok();
}

FResult FPropertyReader::FStructMapAccess::ReadKey(FVisitor &Visitor)
{
	if (Link != nullptr)
	{
		return Visitor.VisitName(Link->GetFName());
	}
	else
	{
		return Fail(EErrorCode::OutOfBoundMapKeyRead);
	}
}

FResult FPropertyReader::FStructMapAccess::ReadValue(FVisitor &Visitor)
{
	if (Link != nullptr)
	{
		auto& StructRootState = GetState<StateStructRoot>(&Parent);
		FPropertyReader ValueReader(
			StructRootState.StructPtr,
			StructRootState.StructClass,
			Link
		);
		return ValueReader.ReadAny(Visitor);
	}
	else
	{
		return Fail(EErrorCode::OutOfBoundMapValueRead);
	}
}

FResult FPropertyReader::FStructMapAccess::Next()
{
	Link = NextEffectiveProperty(Link);
	return Ok();
}

FPropertyReader::FClassMapAccess::FClassMapAccess(FPropertyReader* ParentPtr)
	: Parent(*ParentPtr)
{
	auto Class = GetState<StateClassRoot>(&Parent).ClassObject->GetClass();
	Link = FirstEffectiveProperty(Class->PropertyLink);
}

FResult FPropertyReader::FClassMapAccess::Num(TOptional<size_t>& OutNum)
{
	return Ok();
}

FResult FPropertyReader::FClassMapAccess::HasPending(bool& bOutHasPending)
{
	bOutHasPending = Link != nullptr;
	return Ok();
}

FResult FPropertyReader::FClassMapAccess::Next()
{
	Link = NextEffectiveProperty(Link);
	return Ok();
}

FResult FPropertyReader::FClassMapAccess::ReadKey(FVisitor &Visitor)
{
	if (Link != nullptr)
	{
		return Visitor.VisitName(Link->GetFName());
	}
	else
	{
		return Fail(EErrorCode::OutOfBoundMapKeyRead);
	}
}

FResult FPropertyReader::FClassMapAccess::ReadValue(FVisitor &Visitor)
{
	if (Link != nullptr)
	{
		auto& ClassRootState = GetState<StateClassRoot>(&Parent);
		FPropertyReader ValueReader(
			ClassRootState.ClassObject,
			Link
		);
		return ValueReader.ReadAny(Visitor);
	}
	else
	{
		return Fail(EErrorCode::OutOfBoundMapValueRead);
	}
}

} // namespace DataConfig
