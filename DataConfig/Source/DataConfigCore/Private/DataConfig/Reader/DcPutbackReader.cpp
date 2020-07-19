#include "DataConfig/Reader/DcPutbackReader.h"

namespace DataConfig
{

template<typename TData>
FORCEINLINE_DEBUGGABLE FResult TryUseCachedValue(FPutbackReader* Self, TData* OutPtr)
{
	check(Self->Cached.Num() > 0);

	FDataVariant Value = Self->Cached.Pop();
	if (Value.DataType != TDataEntryType<TData>::Value)
		return Fail(EErrorCode::UnknownError);

	if (OutPtr)
		*OutPtr = Value.GetValue<TData>();

	return Ok();
}

EDataEntry FPutbackReader::Peek()
{
	if (Cached.Num() > 0)
	{
		return Cached.Last().DataType;
	}
	else
	{
		return Reader->Peek();
	}
}

FResult FPutbackReader::ReadNil(FContextStorage* CtxPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<nullptr_t>(this, nullptr);
	}
	else
	{
		return Reader->ReadNil(CtxPtr);
	}
}

FResult FPutbackReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<bool>(this, OutPtr);
	}
	else
	{
		return Reader->ReadBool(OutPtr, CtxPtr);
	}
}

FResult FPutbackReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<FName>(this, OutPtr);
	}
	else
	{
		return Reader->ReadName(OutPtr, CtxPtr);
	}
}

FResult FPutbackReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<FString>(this, OutPtr);
	}
	else
	{
		return Reader->ReadString(OutPtr, CtxPtr);
	}
}

FResult FPutbackReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Reader->ReadStructRoot(OutNamePtr, CtxPtr);
}

FResult FPutbackReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Reader->ReadStructEnd(OutNamePtr, CtxPtr);
}

FResult FPutbackReader::ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	return Reader->ReadClassRoot(OutClassPtr, CtxPtr);
}

FResult FPutbackReader::ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	return Reader->ReadClassEnd(OutClassPtr, CtxPtr);
}

FResult FPutbackReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	return Reader->ReadMapRoot(CtxPtr);
}

FResult FPutbackReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	return Reader->ReadMapEnd(CtxPtr);
}

FResult FPutbackReader::ReadArrayRoot(FContextStorage* CtxPtr)
{
	return Reader->ReadArrayRoot(CtxPtr);
}

FResult FPutbackReader::ReadArrayEnd(FContextStorage* CtxPtr)
{
	return Reader->ReadArrayEnd(CtxPtr);
}

FResult FPutbackReader::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr)
{
	return Reader->ReadReference(OutPtr, CtxPtr);
}

}	// namespace DataConfig



