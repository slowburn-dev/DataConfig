#include "DataConfig/Writer/DcPutbackWriter.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

namespace DcPutbackWriterDetails
{

template<EDcDataEntry Next, typename TMethod, typename... TArgs>
FORCEINLINE FDcResult CachedWrite(FDcPutbackWriter* Self, TMethod Method, TArgs&&... Args)
{
	if (Self->Cached.Num() > 0)
	{
		EDcDataEntry Entry = Self->Cached.Pop();
		if (Next == Entry)
		{
			return DcOk();
		}
		else
		{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< Next << Entry;
		}
	}
	else
	{
		return (Self->Writer->*Method)(Forward<TArgs>(Args)...);
	}
}
	
} // namespace DcPutbackWriterDetails

FDcResult FDcPutbackWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (Cached.Num() > 0)
	{
		return ReadOutOk(bOutOk, Cached.Last() == Next);
	}
	else
	{
		return Writer->PeekWrite(Next, bOutOk);
	}
}

FDcResult FDcPutbackWriter::WriteNil()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Nil>(this, &FDcWriter::WriteNil);
}

FDcResult FDcPutbackWriter::WriteBool(bool Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Bool>(this, &FDcWriter::WriteBool, Value);
}

FDcResult FDcPutbackWriter::WriteName(const FName& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Name>(this, &FDcWriter::WriteName, Value);
}

FDcResult FDcPutbackWriter::WriteString(const FString& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::String>(this, &FDcWriter::WriteString, Value);
}

FDcResult FDcPutbackWriter::WriteText(const FText& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Text>(this, &FDcWriter::WriteText, Value);
}

FDcResult FDcPutbackWriter::WriteEnum(const FDcEnumData& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Enum>(this, &FDcWriter::WriteEnum, Value);
}

FDcResult FDcPutbackWriter::WriteStructRootAccess(FDcStructAccess& Access)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::StructRoot>(this, &FDcWriter::WriteStructRootAccess, Access);
}

FDcResult FDcPutbackWriter::WriteStructEndAccess(FDcStructAccess& Access)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::StructEnd>(this, &FDcWriter::WriteStructEndAccess, Access);
}

FDcResult FDcPutbackWriter::WriteClassRootAccess(FDcClassAccess& Access)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::ClassRoot>(this, &FDcWriter::WriteClassRootAccess, Access);
}

FDcResult FDcPutbackWriter::WriteClassEndAccess(FDcClassAccess& Access)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::ClassEnd>(this, &FDcWriter::WriteClassEndAccess, Access);
}

FDcResult FDcPutbackWriter::WriteMapRoot()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::MapRoot>(this, &FDcWriter::WriteMapRoot);
}

FDcResult FDcPutbackWriter::WriteMapEnd()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::MapEnd>(this, &FDcWriter::WriteMapEnd);
}

FDcResult FDcPutbackWriter::WriteArrayRoot()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::ArrayRoot>(this, &FDcWriter::WriteArrayRoot);
}

FDcResult FDcPutbackWriter::WriteArrayEnd()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::ArrayEnd>(this, &FDcWriter::WriteArrayEnd);
}

FDcResult FDcPutbackWriter::WriteSetRoot()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::SetRoot>(this, &FDcWriter::WriteSetRoot);
}

FDcResult FDcPutbackWriter::WriteSetEnd()
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::SetEnd>(this, &FDcWriter::WriteSetEnd);
}

FDcResult FDcPutbackWriter::WriteObjectReference(const UObject* Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::ObjectReference>(this, &FDcWriter::WriteObjectReference, Value);
}

FDcResult FDcPutbackWriter::WriteClassReference(const UClass* Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::ClassReference>(this, &FDcWriter::WriteClassReference, Value);
}

FDcResult FDcPutbackWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::WeakObjectReference>(this, &FDcWriter::WriteWeakObjectReference, Value);
}

FDcResult FDcPutbackWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::LazyObjectReference>(this, &FDcWriter::WriteLazyObjectReference, Value);
}

FDcResult FDcPutbackWriter::WriteSoftObjectReference(const FSoftObjectPtr& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::SoftObjectReference>(this, &FDcWriter::WriteSoftObjectReference, Value);
}

FDcResult FDcPutbackWriter::WriteSoftClassReference(const FSoftObjectPtr& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::SoftClassReference>(this, &FDcWriter::WriteSoftClassReference, Value);
}

FDcResult FDcPutbackWriter::WriteInterfaceReference(const FScriptInterface& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::InterfaceReference>(this, &FDcWriter::WriteInterfaceReference, Value);
}

FDcResult FDcPutbackWriter::WriteFieldPath(const FFieldPath& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::FieldPath>(this, &FDcWriter::WriteFieldPath, Value);
}

FDcResult FDcPutbackWriter::WriteDelegate(const FScriptDelegate& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Delegate>(this, &FDcWriter::WriteDelegate, Value);
}

FDcResult FDcPutbackWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::MulticastInlineDelegate>(this, &FDcWriter::WriteMulticastInlineDelegate, Value);
}

FDcResult FDcPutbackWriter::WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::MulticastSparseDelegate>(this, &FDcWriter::WriteMulticastSparseDelegate, Value);
}

FDcResult FDcPutbackWriter::WriteInt8(const int8& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Int8>(this, &FDcWriter::WriteInt8, Value);
}

FDcResult FDcPutbackWriter::WriteInt16(const int16& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Int16>(this, &FDcWriter::WriteInt16, Value);
}

FDcResult FDcPutbackWriter::WriteInt32(const int32& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Int32>(this, &FDcWriter::WriteInt32, Value);
}

FDcResult FDcPutbackWriter::WriteInt64(const int64& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Int64>(this, &FDcWriter::WriteInt64, Value);
}

FDcResult FDcPutbackWriter::WriteUInt8(const uint8& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::UInt8>(this, &FDcWriter::WriteUInt8, Value);
}

FDcResult FDcPutbackWriter::WriteUInt16(const uint16& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::UInt16>(this, &FDcWriter::WriteUInt16, Value);
}

FDcResult FDcPutbackWriter::WriteUInt32(const uint32& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::UInt32>(this, &FDcWriter::WriteUInt32, Value);
}

FDcResult FDcPutbackWriter::WriteUInt64(const uint64& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::UInt64>(this, &FDcWriter::WriteUInt64, Value);
}

FDcResult FDcPutbackWriter::WriteFloat(const float& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Float>(this, &FDcWriter::WriteFloat, Value);
}

FDcResult FDcPutbackWriter::WriteDouble(const double& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Double>(this, &FDcWriter::WriteDouble, Value);
}

FDcResult FDcPutbackWriter::WriteBlob(const FDcBlobViewData& Value)
{
	return DcPutbackWriterDetails::CachedWrite<EDcDataEntry::Blob>(this, &FDcWriter::WriteBlob, Value);
}

void FDcPutbackWriter::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Writer->FormatDiagnostic(Diag);

	if (Cached.Num())
	{
		FDcDiagnosticHighlight Highlight(this, ClassId().ToString());
		Highlight.Formatted = FString::Printf(TEXT("(Putback: %d)"), Cached.Num());
		Diag << MoveTemp(Highlight);
	}
}

FName FDcPutbackWriter::ClassId() { return FName(TEXT("DcPutbackWriter")); }
FName FDcPutbackWriter::GetId() { return ClassId();	}

