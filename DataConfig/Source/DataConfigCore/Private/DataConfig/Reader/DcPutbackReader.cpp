#include "DataConfig/Reader/DcPutbackReader.h"

template<typename TData>
FORCEINLINE_DEBUGGABLE FDcResult TryUseCachedValue(FDcPutbackReader* Self, TData* OutPtr)
{
	check(Self->Cached.Num() > 0);

	FDcDataVariant Value = Self->Cached.Pop();
	EDcDataEntry Expected = TDcDataEntryType<TData>::Value;
	if (Value.DataType != Expected)
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< (int)Expected << (int)Value.DataType;

	if (OutPtr)
		*OutPtr = Value.GetValue<TData>();

	return DcOk();
}

FDcResult FDcPutbackReader::ReadNext(EDcDataEntry* OutPtr)
{
	if (Cached.Num() > 0)
	{
		*OutPtr = Cached.Last().DataType;
		return DcOk();
	}
	else
	{
		return Reader->ReadNext(OutPtr);
	}
}

FDcResult FDcPutbackReader::ReadNil()
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<nullptr_t>(this, nullptr);
	}
	else
	{
		return Reader->ReadNil();
	}
}

FDcResult FDcPutbackReader::ReadBool(bool* OutPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<bool>(this, OutPtr);
	}
	else
	{
		return Reader->ReadBool(OutPtr);
	}
}

FDcResult FDcPutbackReader::ReadName(FName* OutPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<FName>(this, OutPtr);
	}
	else
	{
		return Reader->ReadName(OutPtr);
	}
}

FDcResult FDcPutbackReader::ReadString(FString* OutPtr)
{
	if (Cached.Num() > 0)
	{
		return TryUseCachedValue<FString>(this, OutPtr);
	}
	else
	{
		return Reader->ReadString(OutPtr);
	}
}

FDcResult FDcPutbackReader::ReadStructRoot(FName* OutNamePtr)
{
	return Reader->ReadStructRoot(OutNamePtr);
}

FDcResult FDcPutbackReader::ReadStructEnd(FName* OutNamePtr)
{
	return Reader->ReadStructEnd(OutNamePtr);
}

FDcResult FDcPutbackReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr)
{
	return Reader->ReadClassRoot(OutClassPtr);
}

FDcResult FDcPutbackReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr)
{
	return Reader->ReadClassEnd(OutClassPtr);
}

FDcResult FDcPutbackReader::ReadMapRoot()
{
	return Reader->ReadMapRoot();
}

FDcResult FDcPutbackReader::ReadMapEnd()
{
	return Reader->ReadMapEnd();
}

FDcResult FDcPutbackReader::ReadArrayRoot()
{
	return Reader->ReadArrayRoot();
}

FDcResult FDcPutbackReader::ReadArrayEnd()
{
	return Reader->ReadArrayEnd();
}

FDcResult FDcPutbackReader::ReadReference(UObject** OutPtr)
{
	return Reader->ReadReference(OutPtr);
}


