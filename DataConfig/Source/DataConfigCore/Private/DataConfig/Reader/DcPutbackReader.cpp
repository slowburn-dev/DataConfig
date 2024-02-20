#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"


namespace DcPutbackReaderDetails
{

template<typename TData>
FDcResult PopAndCheckCachedValue(FDcPutbackReader* Self, FDcDataVariant& OutValue)
{
	check(Self->Cached.Num() > 0);

	OutValue = Self->Cached.Pop();
	EDcDataEntry Expected = DcTypeUtils::TDcDataEntryType<TData>::Value;
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
	if (Self->Cached.Num() > 0)
	{
		return TryUseCachedValue<TData>(Self, Forward<TArgs>(Args)...);
	}
	else
	{
		return (Self->Reader->*Method)(Forward<TArgs>(Args)...);
	}
}

template<typename TMethod, typename... TArgs>
FORCEINLINE FDcResult CanNotCachedRead(FDcPutbackReader* Self, EDcDataEntry Entry, TMethod Method, TArgs&&... Args)
{
	if (Self->Cached.Num() > 0)
		return DC_FAIL(DcDReadWrite, CantUsePutbackValue) << Entry;

	return (Self->Reader->*Method)(Forward<TArgs>(Args)...);
}

template<typename TMethod>
FORCEINLINE FDcResult CachedReadDataTypeOnly(FDcPutbackReader* Self, EDcDataEntry Entry, TMethod Method)
{
	if (Self->Cached.Num() > 0)
	{
		FDcDataVariant Value = Self->Cached.Pop();
		check(Value.bDataTypeOnly);
		if (Value.DataType != Entry)
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< Entry << Value.DataType;

		return DcOk();
	}
	else
	{
		return (Self->Reader->*Method)();
	}
}

} // namespace DcPutbackReaderDetails

FDcResult FDcPutbackReader::PeekRead(EDcDataEntry* OutPtr)
{
	if (Cached.Num() > 0)
	{
		return ReadOutOk(OutPtr, Cached.Last().DataType);
	}
	else
	{
		return Reader->PeekRead(OutPtr);
	}
}

FDcResult FDcPutbackReader::ReadNone()
{
	return DcPutbackReaderDetails::CachedRead<nullptr_t>(this, &FDcReader::ReadNone);
}

FDcResult FDcPutbackReader::ReadBool(bool* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<bool>(this, &FDcReader::ReadBool, OutPtr);
}

FDcResult FDcPutbackReader::ReadName(FName* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<FName>(this, &FDcReader::ReadName, OutPtr);
}

FDcResult FDcPutbackReader::ReadString(FString* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<FString>(this, &FDcReader::ReadString, OutPtr);
}

FDcResult FDcPutbackReader::ReadText(FText* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<FText>(this, &FDcReader::ReadText, OutPtr);
}

FDcResult FDcPutbackReader::ReadEnum(FDcEnumData* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<FDcEnumData>(this, &FDcReader::ReadEnum, OutPtr);
}

FDcResult FDcPutbackReader::ReadStructRootAccess(FDcStructAccess& Access)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::StructRoot, &FDcReader::ReadStructRootAccess, Access);
}

FDcResult FDcPutbackReader::ReadStructEndAccess(FDcStructAccess& Access)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::StructEnd, &FDcReader::ReadStructEndAccess, Access);
}

FDcResult FDcPutbackReader::ReadOptionalRoot()
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::OptionalRoot, &FDcReader::ReadOptionalRoot);
}

FDcResult FDcPutbackReader::ReadOptionalEnd()
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::OptionalEnd, &FDcReader::ReadOptionalEnd);
}

FDcResult FDcPutbackReader::ReadClassRootAccess(FDcClassAccess& Access)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::ClassRoot, &FDcReader::ReadClassRootAccess, Access);
}

FDcResult FDcPutbackReader::ReadClassEndAccess(FDcClassAccess& Access)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::ClassEnd, &FDcReader::ReadClassEndAccess, Access);
}

FDcResult FDcPutbackReader::ReadMapRoot()
{
	return DcPutbackReaderDetails::CachedReadDataTypeOnly(this, EDcDataEntry::MapRoot, &FDcReader::ReadMapRoot);
}

FDcResult FDcPutbackReader::ReadMapEnd()
{
	return DcPutbackReaderDetails::CachedReadDataTypeOnly(this, EDcDataEntry::MapEnd, &FDcReader::ReadMapEnd);
}

FDcResult FDcPutbackReader::ReadArrayRoot()
{
	return DcPutbackReaderDetails::CachedReadDataTypeOnly(this, EDcDataEntry::ArrayRoot, &FDcReader::ReadArrayRoot);
}

FDcResult FDcPutbackReader::ReadArrayEnd()
{
	return DcPutbackReaderDetails::CachedReadDataTypeOnly(this, EDcDataEntry::ArrayEnd, &FDcReader::ReadArrayEnd);
}

FDcResult FDcPutbackReader::ReadObjectReference(UObject** OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::ObjectReference, &FDcReader::ReadObjectReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadClassReference(UClass** OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::ClassReference, &FDcReader::ReadClassReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadWeakObjectReference(FWeakObjectPtr* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::WeakObjectReference, &FDcReader::ReadWeakObjectReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadLazyObjectReference(FLazyObjectPtr* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::LazyObjectReference, &FDcReader::ReadLazyObjectReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadSoftObjectReference(FSoftObjectPtr* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::SoftObjectReference, &FDcReader::ReadSoftObjectReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadSoftClassReference(FSoftObjectPtr* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::SoftClassReference, &FDcReader::ReadSoftClassReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadInterfaceReference(FScriptInterface* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::InterfaceReference, &FDcReader::ReadInterfaceReference, OutPtr);
}

FDcResult FDcPutbackReader::ReadFieldPath(FFieldPath* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::FieldPath, &FDcReader::ReadFieldPath, OutPtr);
}

FDcResult FDcPutbackReader::ReadDelegate(FScriptDelegate* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::Delegate, &FDcReader::ReadDelegate, OutPtr);
}

FDcResult FDcPutbackReader::ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::MulticastInlineDelegate, &FDcReader::ReadMulticastInlineDelegate, OutPtr);
}

FDcResult FDcPutbackReader::ReadMulticastSparseDelegate(FMulticastScriptDelegate* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::MulticastSparseDelegate, &FDcReader::ReadMulticastSparseDelegate, OutPtr);
}

FDcResult FDcPutbackReader::ReadSetRoot()
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::SetRoot, &FDcReader::ReadSetRoot);
}

FDcResult FDcPutbackReader::ReadSetEnd()
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::SetEnd, &FDcReader::ReadSetEnd);
}

FDcResult FDcPutbackReader::ReadInt8(int8* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<int8>(this, &FDcReader::ReadInt8, OutPtr);
}

FDcResult FDcPutbackReader::ReadInt16(int16* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<int16>(this, &FDcReader::ReadInt16, OutPtr);
}

FDcResult FDcPutbackReader::ReadInt32(int32* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<int32>(this, &FDcReader::ReadInt32, OutPtr);
}

FDcResult FDcPutbackReader::ReadInt64(int64* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<int64>(this, &FDcReader::ReadInt64, OutPtr);
}

FDcResult FDcPutbackReader::ReadUInt8(uint8* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<uint8>(this, &FDcReader::ReadUInt8, OutPtr);
}

FDcResult FDcPutbackReader::ReadUInt16(uint16* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<uint16>(this, &FDcReader::ReadUInt16, OutPtr);
}

FDcResult FDcPutbackReader::ReadUInt32(uint32* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<uint32>(this, &FDcReader::ReadUInt32, OutPtr);
}

FDcResult FDcPutbackReader::ReadUInt64(uint64* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<uint64>(this, &FDcReader::ReadUInt64, OutPtr);
}

FDcResult FDcPutbackReader::ReadFloat(float* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<float>(this, &FDcReader::ReadFloat, OutPtr);
}

FDcResult FDcPutbackReader::ReadDouble(double* OutPtr)
{
	return DcPutbackReaderDetails::CachedRead<double>(this, &FDcReader::ReadDouble, OutPtr);
}

FDcResult FDcPutbackReader::ReadBlob(FDcBlobViewData* OutPtr)
{
	return DcPutbackReaderDetails::CanNotCachedRead(this, EDcDataEntry::Blob, &FDcReader::ReadBlob, OutPtr);
}

FDcResult FDcPutbackReader::Coercion(EDcDataEntry ToEntry, bool* OutPtr)
{
	if (Cached.Num())
		return ReadOutOk(OutPtr, false);

	return Reader->Coercion(ToEntry, OutPtr);
}

void FDcPutbackReader::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Reader->FormatDiagnostic(Diag);

	if (Cached.Num())
	{
		FDcDiagnosticHighlight Highlight(this, ClassId().ToString());
		Highlight.Formatted = FString::Printf(TEXT("(Putback: %d)"), Cached.Num());
		Diag << MoveTemp(Highlight);
	}
}

FName FDcPutbackReader::ClassId() { return FName(TEXT("DcPutbackReader")); }
FName FDcPutbackReader::GetId() { return ClassId(); }
