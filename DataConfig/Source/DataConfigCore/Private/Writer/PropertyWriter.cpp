#include "Writer/PropertyWriter.h"

namespace DataConfig {

static FORCEINLINE WriterState& GetState(FPropertyWriter* Self)
{
	return *((WriterState*)(&Self->ImplStorage));
}

template<typename TState>
static TState& GetState(FPropertyWriter* Self) {
	return GetState(Self).Get<TState>();
}

template<typename TState>
static TState* TryGetState(FPropertyWriter* Self) {
	return GetState(Self).TryGet<TState>();
}

FPropertyWriter::FPropertyWriter()
{
	static_assert(sizeof(FPropertyReader) <= sizeof(FWriterStorage), "larger than cacheline");
	static_assert(sizeof(WriterState) <= sizeof(ImplStorageType), "impl storage size too small");
	new(&ImplStorage) WriterState(TInPlaceType<StateUnknown>{});
}

template<typename TPrimitive, typename TProperty, EErrorCode ErrCode>
FResult TryWritePrimitive(FPropertyWriter* Writer, const TPrimitive& Value)
{
	if (GetState(Writer).IsType<StateEnded>())
		return Fail(EErrorCode::WriteAfterEnded);

	if (StateClassProperty* CPStatePtr = TryGetState<StateClassProperty>(Writer))
	{
		if (TProperty* WriteProperty = Cast<TProperty>(CPStatePtr->Property))
		{
			WriteProperty->SetPropertyValue(CPStatePtr->ClassObject, Value);
			return Ok();
		}
	}
	else if (StateStructProperty* SPStatePtr = TryGetState<StateStructProperty>(Writer))
	{
		if (TProperty* WriteProperty = Cast<TProperty>(SPStatePtr->Property))
		{
			WriteProperty->SetPropertyValue(SPStatePtr->StructPtr, Value);
			return Ok();
		}
	}
	else if (StatePrimitive* PStatePtr = TryGetState<StatePrimitive>(Writer))
	{
		if (TProperty* WriteProperty = Cast<TProperty>(PStatePtr->Property))
		{
			WriteProperty->SetPropertyValue(PStatePtr->PrimitivePtr, Value);
			return Ok();
		}
	}

	return Fail(ErrCode);
}

FResult FPropertyWriter::WriteBool(bool Value)
{
	return TryWritePrimitive<bool, UBoolProperty, EErrorCode::ExpectBoolFail>(this, Value);
}

FResult FPropertyWriter::WriteName(const FName& Value)
{
	return TryWritePrimitive<FName, UNameProperty, EErrorCode::ExpectNameFail>(this, Value);
}

FResult FPropertyWriter::WriteString(const FString& Value)
{
	return TryWritePrimitive<FString, UStrProperty, EErrorCode::ExpectStringFail>(this, Value);
}

FPropertyWriter::~FPropertyWriter()
{
	GetState(this).~WriterState();
}

FPropertyWriter::FPropertyWriter(UObject* ClassObject)
{
	GetState(this).Emplace<StateClassRoot>(ClassObject);
}

FPropertyWriter::FPropertyWriter(UObject* ClassObject, UProperty* Property)
{
	GetState(this).Emplace<StateClassProperty>(ClassObject, Property);
}

FPropertyWriter::FPropertyWriter(void* StructPtr, UScriptStruct* StructClass)
{
	GetState(this).Emplace<StateStructRoot>(StructPtr, StructClass);
}

FPropertyWriter::FPropertyWriter(void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
{
	GetState(this).Emplace<StateStructProperty>(StructPtr, StructClass, Property);
}

FPropertyWriter::FPropertyWriter(void* PrimitivePtr, UProperty* Property)
{
	GetState(this).Emplace<StatePrimitive>(PrimitivePtr, Property);
}


FResult FPropertyWriter::FStructMapWriter::End()
{
	GetState(this).Emplace<StateEnded>();
	return Ok();
}

FPropertyWriter::FStructMapWriter::FStructMapWriter(void* StructPtr, UScriptStruct* StructClass)
	: FPropertyWriter(StructPtr, StructClass)
{
	static_assert(sizeof(FStructMapWriter) <= sizeof(FWriterStorage), "larger than cacheline");
}

FResult FPropertyWriter::WriteStruct(const FName& StructName, FWriterStorage& OutWriter)
{
	if (StateStructRoot* StatePtr = TryGetState<StateStructRoot>(this))
	{
		OutWriter.Emplace<FStructMapWriter>(StatePtr->StructPtr, StatePtr->StructClass);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ExpectStructFail);
	}
}


} // namespace DataConfig
