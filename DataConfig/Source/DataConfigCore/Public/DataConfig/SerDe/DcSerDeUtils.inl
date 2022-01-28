#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/Writer/DcPutbackWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Nil(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadNil());
	DC_TRY(Writer->WriteNil());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Bool(TReader* Reader, TWriter* Writer)
{
	bool Value;
	DC_TRY(Reader->ReadBool(&Value));
	DC_TRY(Writer->WriteBool(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Name(TReader* Reader, TWriter* Writer)
{
	FName Value;
	DC_TRY(Reader->ReadName(&Value));
	DC_TRY(Writer->WriteName(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_String(TReader* Reader, TWriter* Writer)
{
	FString Value;
	DC_TRY(Reader->ReadString(&Value));
	DC_TRY(Writer->WriteString(MoveTemp(Value)));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Text(TReader* Reader, TWriter* Writer)
{
	FText Value;
	DC_TRY(Reader->ReadText(&Value));
	DC_TRY(Writer->WriteText(MoveTemp(Value)));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Enum(TReader* Reader, TWriter* Writer)
{
	FDcEnumData Value;
	DC_TRY(Reader->ReadEnum(&Value));
	DC_TRY(Writer->WriteEnum(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_StructRoot(TReader* Reader, TWriter* Writer)
{
	FDcStructAccess Access;
	DC_TRY(Reader->ReadStructRootAccess(Access));
	DC_TRY(Writer->WriteStructRootAccess(Access));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_StructEnd(TReader* Reader, TWriter* Writer)
{
	FDcStructAccess Access;
	DC_TRY(Reader->ReadStructEndAccess(Access));
	DC_TRY(Writer->WriteStructEndAccess(Access));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_MapRoot(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadMapRoot());
	DC_TRY(Writer->WriteMapRoot());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_MapEnd(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadMapEnd());
	DC_TRY(Writer->WriteMapEnd());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_ClassRoot(TReader* Reader, TWriter* Writer)
{
	FDcClassAccess Access;
	DC_TRY(Reader->ReadClassRootAccess(Access));
	DC_TRY(Writer->WriteClassRootAccess(Access));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_ClassEnd(TReader* Reader, TWriter* Writer)
{
	FDcClassAccess Access;
	DC_TRY(Reader->ReadClassEndAccess(Access));
	DC_TRY(Writer->WriteClassEndAccess(Access));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_ArrayRoot(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadArrayRoot());
	DC_TRY(Writer->WriteArrayRoot());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_ArrayEnd(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadArrayEnd());
	DC_TRY(Writer->WriteArrayEnd());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_SetRoot(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadSetRoot());
	DC_TRY(Writer->WriteSetRoot());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_SetEnd(TReader* Reader, TWriter* Writer)
{
	DC_TRY(Reader->ReadSetEnd());
	DC_TRY(Writer->WriteSetEnd());
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_ObjectReference(TReader* Reader, TWriter* Writer)
{
	UObject* Value;
	DC_TRY(Reader->ReadObjectReference(&Value));
	DC_TRY(Writer->WriteObjectReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_ClassReference(TReader* Reader, TWriter* Writer)
{
	UClass* Value;
	DC_TRY(Reader->ReadClassReference(&Value));
	DC_TRY(Writer->WriteClassReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_WeakObjectReference(TReader* Reader, TWriter* Writer)
{
	FWeakObjectPtr Value;
	DC_TRY(Reader->ReadWeakObjectReference(&Value));
	DC_TRY(Writer->WriteWeakObjectReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_LazyObjectReference(TReader* Reader, TWriter* Writer)
{
	FLazyObjectPtr Value;
	DC_TRY(Reader->ReadLazyObjectReference(&Value));
	DC_TRY(Writer->WriteLazyObjectReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_SoftObjectReference(TReader* Reader, TWriter* Writer)
{
	FSoftObjectPtr Value;
	DC_TRY(Reader->ReadSoftObjectReference(&Value));
	DC_TRY(Writer->WriteSoftObjectReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_SoftClassReference(TReader* Reader, TWriter* Writer)
{
	FSoftObjectPtr Value;
	DC_TRY(Reader->ReadSoftClassReference(&Value));
	DC_TRY(Writer->WriteSoftClassReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_InterfaceReference(TReader* Reader, TWriter* Writer)
{
	FScriptInterface Value;
	DC_TRY(Reader->ReadInterfaceReference(&Value));
	DC_TRY(Writer->WriteInterfaceReference(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_FieldPath(TReader* Reader, TWriter* Writer)
{
	FFieldPath Value;
	DC_TRY(Reader->ReadFieldPath(&Value));
	DC_TRY(Writer->WriteFieldPath(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Delegate(TReader* Reader, TWriter* Writer)
{
	FScriptDelegate Value;
	DC_TRY(Reader->ReadDelegate(&Value));
	DC_TRY(Writer->WriteDelegate(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_MulticastInlineDelegate(TReader* Reader, TWriter* Writer)
{
	FMulticastScriptDelegate Value;
	DC_TRY(Reader->ReadMulticastInlineDelegate(&Value));
	DC_TRY(Writer->WriteMulticastInlineDelegate(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_MulticastSparseDelegate(TReader* Reader, TWriter* Writer)
{
	FMulticastScriptDelegate Value;
	DC_TRY(Reader->ReadMulticastSparseDelegate(&Value));
	DC_TRY(Writer->WriteMulticastSparseDelegate(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Int8(TReader* Reader, TWriter* Writer)
{
	int8 Value;
	DC_TRY(Reader->ReadInt8(&Value));
	DC_TRY(Writer->WriteInt8(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Int16(TReader* Reader, TWriter* Writer)
{
	int16 Value;
	DC_TRY(Reader->ReadInt16(&Value));
	DC_TRY(Writer->WriteInt16(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Int32(TReader* Reader, TWriter* Writer)
{
	int32 Value;
	DC_TRY(Reader->ReadInt32(&Value));
	DC_TRY(Writer->WriteInt32(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Int64(TReader* Reader, TWriter* Writer)
{
	int64 Value;
	DC_TRY(Reader->ReadInt64(&Value));
	DC_TRY(Writer->WriteInt64(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_UInt8(TReader* Reader, TWriter* Writer)
{
	uint8 Value;
	DC_TRY(Reader->ReadUInt8(&Value));
	DC_TRY(Writer->WriteUInt8(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_UInt16(TReader* Reader, TWriter* Writer)
{
	uint16 Value;
	DC_TRY(Reader->ReadUInt16(&Value));
	DC_TRY(Writer->WriteUInt16(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_UInt32(TReader* Reader, TWriter* Writer)
{
	uint32 Value;
	DC_TRY(Reader->ReadUInt32(&Value));
	DC_TRY(Writer->WriteUInt32(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_UInt64(TReader* Reader, TWriter* Writer)
{
	uint64 Value;
	DC_TRY(Reader->ReadUInt64(&Value));
	DC_TRY(Writer->WriteUInt64(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Float(TReader* Reader, TWriter* Writer)
{
	float Value;
	DC_TRY(Reader->ReadFloat(&Value));
	DC_TRY(Writer->WriteFloat(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Double(TReader* Reader, TWriter* Writer)
{
	double Value;
	DC_TRY(Reader->ReadDouble(&Value));
	DC_TRY(Writer->WriteDouble(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE FDcResult DcPipe_Blob(TReader* Reader, TWriter* Writer)
{
	FDcBlobViewData Value;
	DC_TRY(Reader->ReadBlob(&Value));
	DC_TRY(Writer->WriteBlob(Value));
	return DcOk();
}

template<typename TReader, typename TWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcPipe_Dispatch(EDcDataEntry Next, TReader* Reader, TWriter* Writer)
{
	switch (Next)
	{
		case EDcDataEntry::Nil:
			return DcPipe_Nil(Reader, Writer);
		case EDcDataEntry::Bool:
			return DcPipe_Bool(Reader, Writer);
		case EDcDataEntry::Name:
			return DcPipe_Name(Reader, Writer);
		case EDcDataEntry::String:
			return DcPipe_String(Reader, Writer);
		case EDcDataEntry::Text:
			return DcPipe_Text(Reader, Writer);
		case EDcDataEntry::Enum:
			return DcPipe_Enum(Reader, Writer);
		case EDcDataEntry::Int8:
			return DcPipe_Int8(Reader, Writer);
		case EDcDataEntry::Int16:
			return DcPipe_Int16(Reader, Writer);
		case EDcDataEntry::Int32:
			return DcPipe_Int32(Reader, Writer);
		case EDcDataEntry::Int64:
			return DcPipe_Int64(Reader, Writer);
		case EDcDataEntry::UInt8:
			return DcPipe_UInt8(Reader, Writer);
		case EDcDataEntry::UInt16:
			return DcPipe_UInt16(Reader, Writer);
		case EDcDataEntry::UInt32:
			return DcPipe_UInt32(Reader, Writer);
		case EDcDataEntry::UInt64:
			return DcPipe_UInt64(Reader, Writer);
		case EDcDataEntry::Float:
			return DcPipe_Float(Reader, Writer);
		case EDcDataEntry::Double:
			return DcPipe_Double(Reader, Writer);
		case EDcDataEntry::Blob:
			return DcPipe_Blob(Reader, Writer);
		case EDcDataEntry::StructRoot:
			return DcPipe_StructRoot(Reader, Writer);
		case EDcDataEntry::StructEnd:
			return DcPipe_StructEnd(Reader, Writer);
		case EDcDataEntry::MapRoot:
			return DcPipe_MapRoot(Reader, Writer);
		case EDcDataEntry::MapEnd:
			return DcPipe_MapEnd(Reader, Writer);
		case EDcDataEntry::ClassRoot:
			return DcPipe_ClassRoot(Reader, Writer);
		case EDcDataEntry::ClassEnd:
			return DcPipe_ClassEnd(Reader, Writer);
		case EDcDataEntry::ArrayRoot:
			return DcPipe_ArrayRoot(Reader, Writer);
		case EDcDataEntry::ArrayEnd:
			return DcPipe_ArrayEnd(Reader, Writer);
		case EDcDataEntry::SetRoot:
			return DcPipe_SetRoot(Reader, Writer);
		case EDcDataEntry::SetEnd:
			return DcPipe_SetEnd(Reader, Writer);
		case EDcDataEntry::ObjectReference:
			return DcPipe_ObjectReference(Reader, Writer);
		case EDcDataEntry::ClassReference:
			return DcPipe_ClassReference(Reader, Writer);
		case EDcDataEntry::WeakObjectReference:
			return DcPipe_WeakObjectReference(Reader, Writer);
		case EDcDataEntry::LazyObjectReference:
			return DcPipe_LazyObjectReference(Reader, Writer);
		case EDcDataEntry::SoftObjectReference:
			return DcPipe_SoftObjectReference(Reader, Writer);
		case EDcDataEntry::SoftClassReference:
			return DcPipe_SoftClassReference(Reader, Writer);
		case EDcDataEntry::InterfaceReference:
			return DcPipe_InterfaceReference(Reader, Writer);
		case EDcDataEntry::FieldPath:
			return DcPipe_FieldPath(Reader, Writer);
		case EDcDataEntry::Delegate:
			return DcPipe_Delegate(Reader, Writer);
		case EDcDataEntry::MulticastInlineDelegate:
			return DcPipe_MulticastInlineDelegate(Reader, Writer);
		case EDcDataEntry::MulticastSparseDelegate:
			return DcPipe_MulticastSparseDelegate(Reader, Writer);
		case EDcDataEntry::Extension:
			return DC_FAIL(DcDSerDe, PipeUnhandledExtension);
		case EDcDataEntry::Ended:
			return DC_FAIL(DcDSerDe, PipeUnhandledEnded);
		default:
			return DcNoEntry();
	}
	return DcNoEntry();
}

template<typename TReader>
FORCEINLINE FDcResult DcCastReader(FDcReader* Reader, TReader*& OutReader)
{
	if (Reader->GetId() == FDcPutbackReader::ClassId())
		Reader = ((FDcPutbackReader*)Reader)->Reader;

	OutReader = Reader->CastById<TReader>();
	if (OutReader == nullptr)
		return DC_FAIL(DcDReadWrite, ReaderCastExpect)
			<< TReader::ClassId() << Reader->GetId();
	return DcOk();
}

template<typename TWriter>
FORCEINLINE FDcResult DcCastWriter(FDcWriter* Writer, TWriter*& OutWriter)
{
	if (Writer->GetId() == FDcPutbackWriter::ClassId())
		Writer = ((FDcPutbackWriter*)Writer)->Writer;

	OutWriter = Writer->CastById<TWriter>();
	if (OutWriter == nullptr)
		return DC_FAIL(DcDReadWrite, WriterCastExpect)
			<< TWriter::ClassId() << Writer->GetId();
	return DcOk();
}

