#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

template<typename TData>
FORCEINLINE FDcResult TryUseCachedValue(FDcPutbackReader* Self, TData* OutPtr)
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

template<typename TData, typename TMethod, typename... TArgs>
FORCEINLINE FDcResult CachedRead(FDcPutbackReader* Self, TMethod Method, TArgs&&... Args)
{
	Self->QuickSanityCheck();

	if (Self->Cached.Num() > 0)
	{
		return TryUseCachedValue<TData>(Self, Forward<TArgs>(Args)...);
	}
	else
	{
		return (Self->*Method)(Forward<TArgs>(Args)...);
	}
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
		TDcRestore<FDcReader*> NestReader(DcEnv().ActiveReader, Reader);
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
		TDcRestore<FDcReader*> NestReader(DcEnv().ActiveReader, Reader);
		return Reader->ReadNil();
	}
}

FDcResult FDcPutbackReader::ReadBool(bool* OutPtr)
{
	return CachedRead<bool>(this, &FDcReader::ReadBool, OutPtr);
}

FDcResult FDcPutbackReader::ReadName(FName* OutPtr)
{
	return CachedRead<FName>(this, &FDcReader::ReadName, OutPtr);
}

FDcResult FDcPutbackReader::ReadString(FString* OutPtr)
{
	return CachedRead<FString>(this, &FDcReader::ReadString, OutPtr);
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


