#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

template<typename TData>
FDcResult PopAndCheckCachedValue(FDcPutbackReader* Self, FDcDataVariant& OutValue)
{
	check(Self->Cached.Num() > 0);

	OutValue = Self->Cached.Pop();
	EDcDataEntry Expected = TDcDataEntryType<TData>::Value;
	if (OutValue.DataType != Expected)
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< (int)Expected << (int)OutValue.DataType;

	return DcOk();
}

template<typename TData>
FORCEINLINE FDcResult TryUseCachedValue(FDcPutbackReader* Self, TData* OutPtr)
{
	FDcDataVariant Value;
	DC_TRY(PopAndCheckCachedValue<TData>(Self, Value));

	if (OutPtr)
		*OutPtr = Value.GetValue<TData>();

	return DcOk();
}

template<typename TData>
FORCEINLINE FDcResult TryUseCachedValue(FDcPutbackReader* Self)
{
	FDcDataVariant Value;
	DC_TRY(PopAndCheckCachedValue<TData>(Self, Value));
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
		TDcStoreThenReset<FDcReader*> NestReader(DcEnv().ActiveReader, Self->Reader);
		return (Self->*Method)(Forward<TArgs>(Args)...);
	}
}

template<typename TMethod, typename... TArgs>
FORCEINLINE FDcResult CanNotCachedRead(FDcPutbackReader* Self, EDcDataEntry Entry, TMethod Method, TArgs&&... Args)
{
	Self->QuickSanityCheck();
	if (Self->Cached.Num() > 0)
		return DC_FAIL(DcDReadWrite, CantUsePutbackValue) << Entry;

	TDcStoreThenReset<FDcReader*> NestReader(DcEnv().ActiveReader, Self->Reader);
	return (Self->*Method)(Forward<TArgs>(Args)...);
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
		TDcStoreThenReset<FDcReader*> NestReader(DcEnv().ActiveReader, Reader);
		return Reader->ReadNext(OutPtr);
	}
}

FDcResult FDcPutbackReader::ReadNil()
{
	return CachedRead<nullptr_t>(this, &FDcReader::ReadNil);
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
	return CanNotCachedRead(this, EDcDataEntry::StructRoot, &FDcReader::ReadStructRoot, OutNamePtr);
}

FDcResult FDcPutbackReader::ReadStructEnd(FName* OutNamePtr)
{
	return CanNotCachedRead(this, EDcDataEntry::StructEnd, &FDcReader::ReadStructEnd, OutNamePtr);
}

FDcResult FDcPutbackReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr)
{
	return CanNotCachedRead(this, EDcDataEntry::ClassRoot, &FDcReader::ReadClassRoot, OutClassPtr);
}

FDcResult FDcPutbackReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr)
{
	return CanNotCachedRead(this, EDcDataEntry::ClassEnd, &FDcReader::ReadClassEnd, OutClassPtr);
}

FDcResult FDcPutbackReader::ReadMapRoot()
{
	return CanNotCachedRead(this, EDcDataEntry::MapRoot, &FDcReader::ReadMapRoot);
}

FDcResult FDcPutbackReader::ReadMapEnd()
{
	return CanNotCachedRead(this, EDcDataEntry::MapEnd, &FDcReader::ReadMapEnd);
}

FDcResult FDcPutbackReader::ReadArrayRoot()
{
	return CanNotCachedRead(this, EDcDataEntry::ArrayRoot, &FDcReader::ReadArrayRoot);
}

FDcResult FDcPutbackReader::ReadArrayEnd()
{
	return CanNotCachedRead(this, EDcDataEntry::ArrayEnd, &FDcReader::ReadArrayEnd);
}

FDcResult FDcPutbackReader::ReadReference(UObject** OutPtr)
{
	return CanNotCachedRead(this, EDcDataEntry::Reference, &FDcReader::ReadReference, OutPtr);
}


