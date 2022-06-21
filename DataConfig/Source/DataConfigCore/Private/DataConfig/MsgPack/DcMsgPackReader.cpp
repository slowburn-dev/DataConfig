#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackCommon.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticMsgPack.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Misc/DcTypeUtils.h"

namespace DcMsgPackReaderDetails
{

FORCEINLINE_DEBUGGABLE FDcResult CheckNoEOF(FDcMsgPackReader* Self, int32 LookAhead = 1)
{
	if (Self->State.Index + (LookAhead - 1) >= Self->View.Num)
		return DC_FAIL(DcDMsgPack, ReadingPastEnd);
	return DcOk();
}

FORCEINLINE FDcResult Read1(FDcMsgPackReader* Self, uint8* OutPtr)
{
	DC_TRY(CheckNoEOF(Self));
	*OutPtr = Self->View.Get(Self->State.Index);
	Self->State.Index += 1;
	return DcOk();
}

template<int N>
FORCEINLINE FDcResult ReadNRaw(FDcMsgPackReader* Self, FDcFixedBytes<N>* OutBytes)
{
	DC_TRY(CheckNoEOF(Self, N));
	FMemory::Memcpy(OutBytes->Data, Self->View.GetPtr(Self->State.Index), N);
	Self->State.Index += N;

	return DcOk();
}

template<int N>
FORCEINLINE FDcResult ReadN(FDcMsgPackReader* Self, FDcFixedBytes<N>* OutBytes)
{
	DC_TRY(ReadNRaw(Self, OutBytes));

#if PLATFORM_LITTLE_ENDIAN
	DcMsgPackCommon::ReverseBytes<N>(OutBytes->Data);
#endif

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult ReadTypeByte(FDcMsgPackReader* Self, uint8* OutPtr)
{
	DC_TRY(Read1(Self, OutPtr));
	Self->State.LastTypeByte = *OutPtr;
	return DcOk();
}

FORCEINLINE FDcResult ReadTypeByteCheck(FDcMsgPackReader* Self, uint8 Expect)
{
	uint8 TypeByte;
	DC_TRY(ReadTypeByte(Self, &TypeByte));
	if (TypeByte != Expect)
		return DC_FAIL(DcDMsgPack, TypeByteMismatch)
			<< Expect << TypeByte;

	return DcOk();
}

FORCEINLINE FDcResult ReadDataType(FDcMsgPackReader* Self, EDcDataEntry* OutPtr)
{
	uint8 TypeByte;
	DC_TRY(ReadTypeByte(Self, &TypeByte));
	return ReadOutOk(OutPtr, DcMsgPackCommon::TypeByteToDataEntry(TypeByte));
}

FORCEINLINE FDcResult ReadDataTypeCheck(FDcMsgPackReader* Self, EDcDataEntry Expect)
{
	EDcDataEntry Actual;
	DC_TRY(ReadDataType(Self, &Actual));
	if (Actual != Expect)
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< Expect << Actual;

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult EndTopRead(FDcMsgPackReader* Self)
{
	FDcMsgPackReader::FReadState& TopState = Self->States.Top();
	if (TopState.Type == FDcMsgPackReader::EReadState::Array)
	{
		--TopState.Remain;
	}
	else if (TopState.Type == FDcMsgPackReader::EReadState::Map)
	{
		if (TopState.bMapAtValue)
		{
			--TopState.Remain;
			TopState.bMapAtValue = false;
		}
		else
		{
			TopState.bMapAtValue = true;
		}
	}
	else if (TopState.Type == FDcMsgPackReader::EReadState::Root)
	{
		// pass
	}
	else
	{
		return DcNoEntry();
	}

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult CheckTopStateRemains(FDcMsgPackReader* Self)
{
	FDcMsgPackReader::FReadState& TopState = Self->States.Top();
	if (TopState.Type == FDcMsgPackReader::EReadState::Array)
	{
		if (TopState.Remain == 0)
			return DC_FAIL(DcDMsgPack, ReadPassArrayEnd);
	}
	else if (TopState.Type == FDcMsgPackReader::EReadState::Map)
	{
		if (TopState.Remain == 0)
			return DC_FAIL(DcDMsgPack, ReadPassMapEnd);
	}
	else if (TopState.Type == FDcMsgPackReader::EReadState::Root)
	{
		//	pass
	}
	else
	{
		return DcNoEntry();
	}

	return DcOk();
}


template<typename TNumeric>
FORCEINLINE FDcResult ReadNumericDispatch(FDcMsgPackReader* Self, TNumeric* OutPtr)
{
	constexpr int N = sizeof(TNumeric);

	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(Self));
	DC_TRY(ReadDataTypeCheck(Self, DcTypeUtils::TDcDataEntryType<TNumeric>::Value));
	FDcFixedBytes<N> Bytes;
	DC_TRY(ReadN(Self, &Bytes));
	ReadOut(OutPtr, Bytes.template As<TNumeric>());
	return EndTopRead(Self);
}

template<uint8 Ext, int N>
FORCEINLINE FDcResult ReadExtDispatch(FDcMsgPackReader* Self, uint8* OutType, FDcFixedBytes<N>* OutBytes)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(Self));
	DC_TRY(ReadTypeByteCheck(Self, Ext));
	uint8 Type;
	DC_TRY(Read1(Self, &Type));
	FDcFixedBytes<N> Bytes;
	DC_TRY(ReadNRaw(Self, &Bytes));

	ReadOut(OutType, Type);
	ReadOut(OutBytes, Bytes);
	return EndTopRead(Self);
}

} // namespace DcMsgPackReaderDetails


FDcMsgPackReader::FDcMsgPackReader()
	: FDcMsgPackReader({nullptr, 0})
{}

FDcMsgPackReader::FDcMsgPackReader(FDcBlobViewData Blob)
{
	View = Blob;
	States.Add({EReadState::Root, false, 0});
}

FDcResult FDcMsgPackReader::PeekRead(EDcDataEntry* OutPtr)
{
	using namespace  DcMsgPackReaderDetails;

	FReadState& TopState = States.Top();
	if (TopState.Type == EReadState::Array
		&& TopState.Remain == 0)
	{
		return ReadOutOk(OutPtr, EDcDataEntry::ArrayEnd);
	}
	else if (TopState.Type == EReadState::Map
		&& TopState.Remain == 0)
	{
		return ReadOutOk(OutPtr, EDcDataEntry::MapEnd);
	}
	else
	{
		if (State.Index >= View.Num)
			return ReadOutOk(OutPtr, EDcDataEntry::Ended);

		uint8 TypeByte = View.Get(State.Index);
		EDcDataEntry Entry = DcMsgPackCommon::TypeByteToDataEntry(TypeByte);

		if (Entry == EDcDataEntry::Ended)
		{
			return DC_FAIL(DcDMsgPack, UnknownMsgTypeByte)
				<< FString::Printf(TEXT("%x"), TypeByte);
		}
		else
		{
			return ReadOutOk(OutPtr, Entry);
		}
	}
}

FDcResult FDcMsgPackReader::Coercion(EDcDataEntry ToEntry, bool* OutPtr)
{
	EDcDataEntry Next;
	DC_TRY(PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		return ReadOutOk(OutPtr, ToEntry == EDcDataEntry::Name
			|| ToEntry == EDcDataEntry::Text);
	}

	return ReadOutOk(OutPtr, false);
}

FDcResult FDcMsgPackReader::PeekTypeByte(uint8* OutPtr)
{
	if (State.Index >= View.Num)
		return DC_FAIL(DcDMsgPack, ReadingPastEnd);

	return ReadOutOk(OutPtr, View.Get(State.Index));
}

FDcResult FDcMsgPackReader::ReadNil()
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));
	DC_TRY(DcMsgPackReaderDetails::ReadDataTypeCheck(this, EDcDataEntry::Nil));
	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadBool(bool* OutPtr)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));
	if (TypeByte == DcMsgPackCommon::_TRUE)
		ReadOut(OutPtr, true);
	else if (TypeByte == DcMsgPackCommon::_FALSE)
		ReadOut(OutPtr, false);
	else
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::Bool << DcMsgPackCommon::TypeByteToDataEntry(TypeByte);

	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadString(FString* OutPtr)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));
	int32 Size;
	if (TypeByte >= DcMsgPackCommon::_MINFIXSTR && TypeByte <= DcMsgPackCommon::_MAXFIXSTR)
	{
		Size = 0b0001'1111 & TypeByte;
	}
	else if (TypeByte == DcMsgPackCommon::_STR8)
	{
		uint8 Byte;
		DC_TRY(DcMsgPackReaderDetails::Read1(this, &Byte));
		Size = Byte;
	}
	else if (TypeByte == DcMsgPackCommon::_STR16)
	{
		FDcBytes2 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		Size = Bytes.As<uint16>();
	}
	else if (TypeByte == DcMsgPackCommon::_STR32)
	{
		FDcBytes4 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		uint32 USize = Bytes.As<uint32>();
		if (USize > (uint32)TNumericLimits<int32>::Max())
			return DC_FAIL(DcDMsgPack, SizeOverInt32Max);

		Size = (int32)USize;
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::String << DcMsgPackCommon::TypeByteToDataEntry(TypeByte);
	}

	DC_TRY(DcMsgPackReaderDetails::CheckNoEOF(this, Size));
	if (OutPtr)
	{
		//	UTF8 conv when detects non ascii chars
		FUTF8ToTCHAR UTF8Conv((const ANSICHAR*)(View.DataPtr + State.Index), Size);
		*OutPtr = FString(UTF8Conv.Length(), UTF8Conv.Get());
	}

	State.Index += Size;
	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadName(FName* OutPtr)
{
	FString Str;
	DC_TRY(ReadString(&Str));

	if (Str.Len() >= NAME_SIZE)
		return DC_FAIL(DcDReadWrite, FNameOverSize);

	if (OutPtr)
		*OutPtr = FName(Str);

	return DcOk();
}

FDcResult FDcMsgPackReader::ReadText(FText* OutPtr)
{
	FString Str;
	DC_TRY(ReadString(&Str));

	if (OutPtr)
		*OutPtr = FText::FromString(Str);

	return DcOk();
}

FDcResult FDcMsgPackReader::ReadBlob(FDcBlobViewData* OutPtr)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));
	int Size;
	if (TypeByte == DcMsgPackCommon::_BIN8)
	{
		uint8 Byte;
		DC_TRY(DcMsgPackReaderDetails::Read1(this, &Byte));
		Size = Byte;
	}
	else if (TypeByte == DcMsgPackCommon::_BIN16)
	{
		FDcBytes2 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		Size = Bytes.As<uint16>();
	}
	else if (TypeByte == DcMsgPackCommon::_BIN32)
	{
		FDcBytes4 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		uint32 USize = Bytes.As<uint32>();
		if (USize > (uint32)TNumericLimits<int32>::Max())
			return DC_FAIL(DcDMsgPack, SizeOverInt32Max);

		Size = (int32)USize;
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::Blob << DcMsgPackCommon::TypeByteToDataEntry(TypeByte);
	}

	DC_TRY(DcMsgPackReaderDetails::CheckNoEOF(this, Size));
	if (OutPtr)
	{
		OutPtr->DataPtr = View.DataPtr + State.Index;
		OutPtr->Num = Size;
	}

	State.Index += Size;
	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadMapRoot()
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));
	int32 Size;
	if (TypeByte >= DcMsgPackCommon::_MINFIXMAP && TypeByte <= DcMsgPackCommon::_MAXFIXMAP)
	{
		Size = 0b1111 & TypeByte;
	}
	else if (TypeByte == DcMsgPackCommon::_MAP16)
	{
		FDcBytes2 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		Size = Bytes.As<uint16>();
	}
	else if (TypeByte == DcMsgPackCommon::_MAP32)
	{
		FDcBytes4 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		uint32 USize = Bytes.As<uint32>();
		if (USize > (uint32)TNumericLimits<int32>::Max())
			return DC_FAIL(DcDMsgPack, SizeOverInt32Max);

		Size = (int32)USize;
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::MapRoot << DcMsgPackCommon::TypeByteToDataEntry(TypeByte);
	}

	States.Add({EReadState::Map, false, Size});
	return DcOk();
}

FDcResult FDcMsgPackReader::ReadMapEnd()
{
	FReadState TopState = States.Pop();
	if (TopState.Type != EReadState::Map
		|| TopState.bMapAtValue)
		return DC_FAIL(DcDMsgPack, UnexpectedMapEnd);

	if (TopState.Remain != 0)
		return DC_FAIL(DcDMsgPack, MapRemains) << TopState.Remain;

	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadArrayRoot()
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));
	int32 Size;
	if (TypeByte >= DcMsgPackCommon::_MINFIXARRAY && TypeByte <= DcMsgPackCommon::_MAXFIXARRAY)
	{
		Size = 0b1111 & TypeByte;
	}
	else if (TypeByte == DcMsgPackCommon::_ARRAY16)
	{
		FDcBytes2 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		Size = Bytes.As<uint16>();
	}
	else if (TypeByte == DcMsgPackCommon::_ARRAY32)
	{
		FDcBytes4 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		uint32 USize = Bytes.As<uint32>();
		if (USize > (uint32)TNumericLimits<int32>::Max())
			return DC_FAIL(DcDMsgPack, SizeOverInt32Max);

		Size = (int32)USize;
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::ArrayRoot << DcMsgPackCommon::TypeByteToDataEntry(TypeByte);
	}

	States.Add({EReadState::Array, false, Size});
	return DcOk();
}

FDcResult FDcMsgPackReader::ReadArrayEnd()
{
	FReadState TopState = States.Pop();
	if (TopState.Type != EReadState::Array)
		return DC_FAIL(DcDMsgPack, UnexpectedArrayEnd);

	if (TopState.Remain != 0)
		return DC_FAIL(DcDMsgPack, ArrayRemains) << TopState.Remain;

	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadInt8(int8* OutPtr)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	using namespace DcMsgPackCommon;

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));

	if (TypeByte >= _MINNEGATIVEFIXINT && TypeByte <= _MAXNEGATIVEFIXINT)
	{
		ReadOut(OutPtr, reinterpret_cast<int8&>(TypeByte));
	}
	else if (TypeByte >= _MINFIXINT && TypeByte <= _MAXFIXINT)
	{
		ReadOut(OutPtr, reinterpret_cast<int8&>(TypeByte));
	}
	else
	{
		uint8 Byte;
		DC_TRY(DcMsgPackReaderDetails::Read1(this, &Byte));
		ReadOut(OutPtr, reinterpret_cast<int8&>(Byte));
	}

	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadInt16(int16* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }
FDcResult FDcMsgPackReader::ReadInt32(int32* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }
FDcResult FDcMsgPackReader::ReadInt64(int64* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }

FDcResult FDcMsgPackReader::ReadUInt8(uint8* OutPtr)
{
	using namespace DcMsgPackCommon;

	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));

	if (TypeByte >= _MINFIXINT && TypeByte <= _MAXFIXINT)
	{
		ReadOut(OutPtr, TypeByte);
	}
	else
	{
		uint8 Byte;
		DC_TRY(DcMsgPackReaderDetails::Read1(this, &Byte));
		ReadOut(OutPtr, Byte);
	}

	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadUInt16(uint16* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }
FDcResult FDcMsgPackReader::ReadUInt32(uint32* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }
FDcResult FDcMsgPackReader::ReadUInt64(uint64* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }

FDcResult FDcMsgPackReader::ReadFloat(float* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }
FDcResult FDcMsgPackReader::ReadDouble(double* OutPtr) { return DcMsgPackReaderDetails::ReadNumericDispatch(this, OutPtr); }


FDcResult FDcMsgPackReader::ReadFixExt1(uint8* OutType, uint8* OutByte)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	DC_TRY(DcMsgPackReaderDetails::ReadTypeByteCheck(this, DcMsgPackCommon::_FIXEXT1));
	uint8 Type;
	DC_TRY(DcMsgPackReaderDetails::Read1(this, &Type));
	uint8 Byte;
	DC_TRY(DcMsgPackReaderDetails::Read1(this, &Byte));

	ReadOut(OutType, Type);
	ReadOut(OutByte, Byte);
	return DcMsgPackReaderDetails::EndTopRead(this);
}

FDcResult FDcMsgPackReader::ReadFixExt2(uint8* OutType, FDcBytes2* OutBytes)
{
	return DcMsgPackReaderDetails::ReadExtDispatch<DcMsgPackCommon::_FIXEXT2>(this, OutType, OutBytes);
}

FDcResult FDcMsgPackReader::ReadFixExt4(uint8* OutType, FDcBytes4* OutBytes)
{
	return DcMsgPackReaderDetails::ReadExtDispatch<DcMsgPackCommon::_FIXEXT4>(this, OutType, OutBytes);
}

FDcResult FDcMsgPackReader::ReadFixExt8(uint8* OutType, FDcBytes8* OutBytes)
{
	return DcMsgPackReaderDetails::ReadExtDispatch<DcMsgPackCommon::_FIXEXT8>(this, OutType, OutBytes);
}

FDcResult FDcMsgPackReader::ReadFixExt16(uint8* OutType, FDcBytes16* OutBytes)
{
	return DcMsgPackReaderDetails::ReadExtDispatch<DcMsgPackCommon::_FIXEXT16>(this, OutType, OutBytes);
}

FDcResult FDcMsgPackReader::ReadExt(uint8* OutType, FDcBlobViewData* OutBlob)
{
	DC_TRY(DcMsgPackReaderDetails::CheckTopStateRemains(this));

	uint8 TypeByte;
	DC_TRY(DcMsgPackReaderDetails::ReadTypeByte(this, &TypeByte));
	int Size;
	if (TypeByte == DcMsgPackCommon::_EXT8)
	{
		uint8 Byte;
		DC_TRY(DcMsgPackReaderDetails::Read1(this, &Byte));
		Size = Byte;
	}
	else if (TypeByte == DcMsgPackCommon::_EXT16)
	{
		FDcBytes2 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		Size = Bytes.As<uint16>();
	}
	else if (TypeByte == DcMsgPackCommon::_EXT32)
	{
		FDcBytes4 Bytes;
		DC_TRY(DcMsgPackReaderDetails::ReadN(this, &Bytes));
		uint32 USize = Bytes.As<uint32>();
		if (USize > (uint32)TNumericLimits<int32>::Max())
			return DC_FAIL(DcDMsgPack, SizeOverInt32Max);

		Size = (int32)USize;
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::Extension << DcMsgPackCommon::TypeByteToDataEntry(TypeByte);
	}

	uint8 Type;
	DC_TRY(DcMsgPackReaderDetails::Read1(this, &Type));
	ReadOut(OutType, Type);

	DC_TRY(DcMsgPackReaderDetails::CheckNoEOF(this, Size));
	if (OutBlob)
	{
		OutBlob->DataPtr = View.DataPtr + State.Index;
		OutBlob->Num = Size;
	}
	State.Index += Size;
	return DcMsgPackReaderDetails::EndTopRead(this);
}

FName FDcMsgPackReader::ClassId() { return FName(TEXT("DcMsgPackReader")); }
FName FDcMsgPackReader::GetId() { return ClassId(); }

void FDcMsgPackReader::FormatDiagnostic(FDcDiagnostic& Diag)
{
	FDcDiagnosticHighlight Highlight(this, ClassId().ToString());

	UEnum* DataEntryEnum = StaticEnum<EDcDataEntry>();
	check(DataEntryEnum);

	Highlight.Formatted = FString::Printf(TEXT("Last read: %s"), 
		*DataEntryEnum->GetNameStringByIndex((int32)DcMsgPackCommon::TypeByteToDataEntry(State.LastTypeByte))
		);

	Diag << MoveTemp(Highlight);
}

