#include "DataConfig/Reader/DcPutbackReader.h"

template<typename TData>
FORCEINLINE_DEBUGGABLE FDcResult TryUseCachedValue(FDcPutbackReader* Self, TData* OutPtr)
{
	check(Self->Cached.Num() > 0);

	FDcDataVariant Value = Self->Cached.Pop();
	EDcDataEntry Expected = TDcDataEntryType<TData>::Value;
	if (Value.DataType != Expected)
		return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
			<< (int)Expected << (int)Value.DataType;

	if (OutPtr)
		*OutPtr = Value.GetValue<TData>();

	return DcOk();
}

EDcDataEntry FDcPutbackReader::Peek()
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

FDcResult FDcPutbackReader::ReadNil(FContextStorage* CtxPtr)
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

FDcResult FDcPutbackReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
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

FDcResult FDcPutbackReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
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

FDcResult FDcPutbackReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
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

FDcResult FDcPutbackReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Reader->ReadStructRoot(OutNamePtr, CtxPtr);
}

FDcResult FDcPutbackReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Reader->ReadStructEnd(OutNamePtr, CtxPtr);
}

FDcResult FDcPutbackReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	return Reader->ReadClassRoot(OutClassPtr, CtxPtr);
}

FDcResult FDcPutbackReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	return Reader->ReadClassEnd(OutClassPtr, CtxPtr);
}

FDcResult FDcPutbackReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	return Reader->ReadMapRoot(CtxPtr);
}

FDcResult FDcPutbackReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	return Reader->ReadMapEnd(CtxPtr);
}

FDcResult FDcPutbackReader::ReadArrayRoot(FContextStorage* CtxPtr)
{
	return Reader->ReadArrayRoot(CtxPtr);
}

FDcResult FDcPutbackReader::ReadArrayEnd(FContextStorage* CtxPtr)
{
	return Reader->ReadArrayEnd(CtxPtr);
}

FDcResult FDcPutbackReader::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr)
{
	return Reader->ReadReference(OutPtr, CtxPtr);
}


