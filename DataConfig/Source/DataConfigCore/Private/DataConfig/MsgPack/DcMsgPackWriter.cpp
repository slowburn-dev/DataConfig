#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/MsgPack/DcMsgPackCommon.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticMsgPack.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

namespace DcMsgPackWriterDetails
{

static FORCEINLINE uint8 Mask_4b_4b(uint8 Lhs, uint8 Rhs)
{
	return (Lhs & 0b1111'0000) | (Rhs & 0b0000'1111);
}

static FORCEINLINE uint8 Mask_3b_5b(uint8 Lhs, uint8 Rhs)
{
	return (Lhs & 0b1110'0000) | (Rhs & 0b0001'1111);
}

template<typename TNumeric>
static FORCEINLINE void WriteNumber(FDcMsgPackWriter::BufferType& Buffer, TNumeric Value)
{
	constexpr int Size = sizeof(TNumeric);
	int StartIx = Buffer.AddUninitialized(Size);
	uint8* StartPtr = Buffer.GetData() + StartIx;
	FPlatformMemory::Memcpy(StartPtr, &Value, Size);

#if PLATFORM_LITTLE_ENDIAN
	DcMsgPackCommon::ReverseBytes<Size>(StartPtr);
#endif
}

static FORCEINLINE_DEBUGGABLE void EndWriteValuePosition(FDcMsgPackWriter* Self)
{
	FDcMsgPackWriter::FWriteState& TopState = Self->States.Top();
	if (TopState.Type == FDcMsgPackWriter::EWriteState::Array)
	{
		++TopState.Size;
	}
	else if (TopState.Type == FDcMsgPackWriter::EWriteState::Map)
	{
		if (TopState.bMapAtValue)
		{
			TopState.bMapAtValue = false;
			++TopState.Size;
		}
		else
		{
			TopState.bMapAtValue = true;
		}
	}
	else
	{
		// pass
	}
}

static FORCEINLINE_DEBUGGABLE void WriteTypeByte(FDcMsgPackWriter::FWriteState& State, uint8 TypeByte)
{
	State.Buffer.Add(TypeByte);
	State.LastTypeByte = TypeByte;
}

template<int N>
static FORCEINLINE void WriteFixExt(FDcMsgPackWriter::BufferType& Buffer, FDcFixedBytes<N> Bytes)
{
	int StartIx = Buffer.AddUninitialized(N);
	uint8* StartPtr = Buffer.GetData() + StartIx;
	FMemory::Memcpy(StartPtr, Bytes.Data, N);
}

FORCEINLINE bool IsValidWriteScalar(EDcDataEntry Entry)
{
	switch (Entry)
	{
		case EDcDataEntry::None:
		case EDcDataEntry::Bool:
		case EDcDataEntry::Name:
		case EDcDataEntry::String:
		case EDcDataEntry::Text:
		case EDcDataEntry::Float:
		case EDcDataEntry::Double:
		case EDcDataEntry::Int8:
		case EDcDataEntry::Int16:
		case EDcDataEntry::Int32:
		case EDcDataEntry::Int64:
		case EDcDataEntry::UInt8:
		case EDcDataEntry::UInt16:
		case EDcDataEntry::UInt32:
		case EDcDataEntry::UInt64:
		case EDcDataEntry::Blob:
		case EDcDataEntry::Extension:
			return true;
		default:
			return false;
	}
}


} // namespace DcMsgPackWriterDetails

FDcMsgPackWriter::BufferType& FDcMsgPackWriter::GetMainBuffer()
{
	return States[0].Buffer;
}

FDcMsgPackWriter::FDcMsgPackWriter()
{
	States.Add({EWriteState::Root, 0});
}

FDcResult FDcMsgPackWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	using namespace DcMsgPackWriterDetails;

	EWriteState WriteState = GetTopStateType();
	if (WriteState == EWriteState::Array)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayEnd
			|| Next == EDcDataEntry::MapRoot
			|| Next == EDcDataEntry::ArrayRoot
			|| IsValidWriteScalar(Next)
		);
	}
	else if (WriteState == EWriteState::Map)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::MapEnd
			|| Next == EDcDataEntry::MapRoot
			|| Next == EDcDataEntry::ArrayRoot
			|| IsValidWriteScalar(Next)
		);
	}
	else if (WriteState == EWriteState::Root)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended
			|| Next == EDcDataEntry::MapRoot
			|| Next == EDcDataEntry::ArrayRoot
			|| IsValidWriteScalar(Next)
		);
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcMsgPackWriter::WriteNone()
{
	DcMsgPackWriterDetails::WriteTypeByte(States.Top(), DcMsgPackCommon::MSGPACK_NIL);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteBool(bool Value)
{
	DcMsgPackWriterDetails::WriteTypeByte(States.Top(), Value
		? DcMsgPackCommon::MSGPACK_TRUE : DcMsgPackCommon::MSGPACK_FALSE
	);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteString(const FString& Value)
{
	FWriteState& TopState = States.Top();
	FTCHARToUTF8 Encoded(*Value);
	int Len = Encoded.Length();
	const uint8* Bytes = (const uint8*)Encoded.Get();
	if (Len <= 0b11111)
	{
		DcMsgPackWriterDetails::WriteTypeByte(
			TopState,
			DcMsgPackWriterDetails::Mask_3b_5b(
				DcMsgPackCommon::MSGPACK_MINFIXSTR,
				(uint8)Len
		));
		TopState.Buffer.Append(Bytes, Len);
	}
	else if (Len <= 0xFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_STR8);
		TopState.Buffer.Add((uint8)Len);
		TopState.Buffer.Append(Bytes, Len);
	}
	else if (Len <= 0xFFFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_STR16);
		DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, (uint16)Len);
		TopState.Buffer.Append(Bytes, Len);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_STR32);
		DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, (uint32)Len);
		TopState.Buffer.Append(Bytes, Len);
	}

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteName(const FName& Name)
{
	return WriteString(Name.ToString());
}

FDcResult FDcMsgPackWriter::WriteText(const FText& Value)
{
	return WriteString(Value.ToString());
}

FDcResult FDcMsgPackWriter::WriteBlob(const FDcBlobViewData& Value)
{
	FWriteState& TopState = States.Top();
	if (Value.Num <= 0xFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_BIN8);
		TopState.Buffer.Add((uint8)Value.Num);
		TopState.Buffer.Append(Value.DataPtr, Value.Num);
	}
	else if (Value.Num <= 0xFFFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_BIN16);
		DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, (uint16)Value.Num);
		TopState.Buffer.Append(Value.DataPtr, Value.Num);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_BIN32);
		DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, (uint32)Value.Num);
		TopState.Buffer.Append(Value.DataPtr, Value.Num);
	}

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteMapRoot()
{
	States.Add({EWriteState::Map, 0, false});
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteMapEnd()
{
	FWriteState& TopState = States.Top();
	if (TopState.Type != EWriteState::Map
		|| TopState.bMapAtValue)
		return DC_FAIL(DcDMsgPack, UnexpectedMapEnd);

	FWriteState& ParentState = States.Last(1);
	if (TopState.Size <= 0b1111)
	{
		DcMsgPackWriterDetails::WriteTypeByte(
			ParentState,
			DcMsgPackWriterDetails::Mask_4b_4b(
				DcMsgPackCommon::MSGPACK_MINFIXMAP,
				(uint8)TopState.Size
		));
	}
	else if (TopState.Size <= 0xFFFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(ParentState, DcMsgPackCommon::MSGPACK_MAP16);
		DcMsgPackWriterDetails::WriteNumber(ParentState.Buffer, (uint16)TopState.Size);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(ParentState, DcMsgPackCommon::MSGPACK_MAP32);
		DcMsgPackWriterDetails::WriteNumber(ParentState.Buffer, (uint32)TopState.Size);
	}

	ParentState.Buffer.Append(TopState.Buffer.GetData(), TopState.Buffer.Num());

	// note buffers can shrink, reassign CurBuffer from top
	States.RemoveAt(States.Num() - 1);

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteArrayRoot()
{
	States.Add({EWriteState::Array, 0});
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteArrayEnd()
{
	FWriteState& TopState = States.Top();
	if (TopState.Type != EWriteState::Array)
		return DC_FAIL(DcDMsgPack, UnexpectedArrayEnd);

	FWriteState& ParentState = States.Last(1);
	if (TopState.Size <= 0b1111)
	{
		DcMsgPackWriterDetails::WriteTypeByte(
			ParentState,
			DcMsgPackWriterDetails::Mask_4b_4b(
				DcMsgPackCommon::MSGPACK_MINFIXARRAY,
				(uint8)TopState.Size
		));
	}
	else if (TopState.Size <= 0xFFFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(ParentState, DcMsgPackCommon::MSGPACK_ARRAY16);
		DcMsgPackWriterDetails::WriteNumber(ParentState.Buffer, (uint16)TopState.Size);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(ParentState, DcMsgPackCommon::MSGPACK_ARRAY32);
		DcMsgPackWriterDetails::WriteNumber(ParentState.Buffer, (uint32)TopState.Size);
	}

	ParentState.Buffer.Append(TopState.Buffer.GetData(), TopState.Buffer.Num());

	// note buffers can shrink, reassign CurBuffer from top
	States.RemoveAt(States.Num() - 1);

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteUInt8(const uint8& Value)
{
	FWriteState& TopState = States.Top();
	if (Value < 128)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, Value);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_UINT8);
		TopState.Buffer.Add(Value);
	}

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteUInt16(const uint16& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_UINT16);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteUInt32(const uint32& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_UINT32);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteUInt64(const uint64& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_UINT64);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteInt8(const int8& Value)
{
	FWriteState& TopState = States.Top();
	if (Value >= -32)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, Value);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_INT8);
		TopState.Buffer.Add(Value);
	}

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteInt16(const int16& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_INT16);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteInt32(const int32& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_INT32);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteInt64(const int64& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_INT64);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteFloat(const float& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FLOAT32);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteDouble(const double& Value)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FLOAT64);
	DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Value);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteFixExt1(uint8 Type, uint8 Byte)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FIXEXT1);
	TopState.Buffer.Add(Type);
	TopState.Buffer.Add(Byte);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteFixExt2(uint8 Type, FDcBytes2 Bytes)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FIXEXT2);
	TopState.Buffer.Add(Type);
	DcMsgPackWriterDetails::WriteFixExt(TopState.Buffer, Bytes);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteFixExt4(uint8 Type, FDcBytes4 Bytes)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FIXEXT4);
	TopState.Buffer.Add(Type);
	DcMsgPackWriterDetails::WriteFixExt(TopState.Buffer, Bytes);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteFixExt8(uint8 Type, FDcBytes8 Bytes)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FIXEXT8);
	TopState.Buffer.Add(Type);
	DcMsgPackWriterDetails::WriteFixExt(TopState.Buffer, Bytes);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

FDcResult FDcMsgPackWriter::WriteFixExt16(uint8 Type, FDcBytes16 Bytes)
{
	FWriteState& TopState = States.Top();
	DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_FIXEXT16);
	TopState.Buffer.Add(Type);
	DcMsgPackWriterDetails::WriteFixExt(TopState.Buffer, Bytes);
	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}


FDcResult FDcMsgPackWriter::WriteExt(uint8 Type, FDcBlobViewData Blob)
{
	FWriteState& TopState = States.Top();
	int Size = Blob.Num;
	if (Size <= 0xFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_EXT8);
		TopState.Buffer.Add(Size);
		TopState.Buffer.Add(Type);
		TopState.Buffer.Append(Blob.DataPtr, Size);
	}
	else if (Size <= 0xFFFF)
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_EXT16);
		DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, (uint16)Size);
		TopState.Buffer.Add(Type);
		TopState.Buffer.Append(Blob.DataPtr, Size);
	}
	else
	{
		DcMsgPackWriterDetails::WriteTypeByte(TopState, DcMsgPackCommon::MSGPACK_EXT32);
		DcMsgPackWriterDetails::WriteNumber(TopState.Buffer, Size);
		TopState.Buffer.Add(Type);
		TopState.Buffer.Append(Blob.DataPtr, Size);
	}

	DcMsgPackWriterDetails::EndWriteValuePosition(this);
	return DcOk();
}

void FDcMsgPackWriter::FormatDiagnostic(FDcDiagnostic& Diag)
{
	FDcDiagnosticHighlight Highlight(this, ClassId().ToString());

	UEnum* DataEntryEnum = StaticEnum<EDcDataEntry>();
	check(DataEntryEnum);

	Highlight.Formatted = FString::Printf(TEXT("Last write: %s"), 
		*DataEntryEnum->GetNameStringByIndex((int32)DcMsgPackCommon::TypeByteToDataEntry(States.Top().LastTypeByte))
		);

	Diag << MoveTemp(Highlight);
}

FName FDcMsgPackWriter::ClassId() { return FName(TEXT("DcMsgPackWriter")); }
FName FDcMsgPackWriter::GetId() { return ClassId(); }



