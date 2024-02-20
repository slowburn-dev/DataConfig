#pragma once

#include "CoreTypes.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"

namespace DcMsgPackCommon
{

static const uint8 MSGPACK_MINFIXINT = 0x00; // 0
static const uint8 MSGPACK_MAXFIXINT = 0x7f; // 127
static const uint8 MSGPACK_MINFIXMAP = 0x80; // 128
static const uint8 MSGPACK_MAXFIXMAP = 0x8f; // 143
static const uint8 MSGPACK_MINFIXARRAY = 0x90; // 144
static const uint8 MSGPACK_MAXFIXARRAY = 0x9f; // 159
static const uint8 MSGPACK_MINFIXSTR = 0xa0; // 160
static const uint8 MSGPACK_MAXFIXSTR = 0xbf; // 191
static const uint8 MSGPACK_NIL = 0xc0;
static const uint8 MSGPACK_NEVERUSED = 0xc1;
static const uint8 MSGPACK_FALSE = 0xc2;
static const uint8 MSGPACK_TRUE = 0xc3;
static const uint8 MSGPACK_BIN8 = 0xc4;
static const uint8 MSGPACK_BIN16 = 0xc5;
static const uint8 MSGPACK_BIN32 = 0xc6;
static const uint8 MSGPACK_EXT8 = 0xc7;
static const uint8 MSGPACK_EXT16 = 0xc8;
static const uint8 MSGPACK_EXT32 = 0xc9;
static const uint8 MSGPACK_FLOAT32 = 0xca;
static const uint8 MSGPACK_FLOAT64 = 0xcb;
static const uint8 MSGPACK_UINT8 = 0xcc;
static const uint8 MSGPACK_UINT16 = 0xcd;
static const uint8 MSGPACK_UINT32 = 0xce;
static const uint8 MSGPACK_UINT64 = 0xcf;
static const uint8 MSGPACK_INT8 = 0xd0;
static const uint8 MSGPACK_INT16 = 0xd1;
static const uint8 MSGPACK_INT32 = 0xd2;
static const uint8 MSGPACK_INT64 = 0xd3;
static const uint8 MSGPACK_FIXEXT1 = 0xd4;
static const uint8 MSGPACK_FIXEXT2 = 0xd5;
static const uint8 MSGPACK_FIXEXT4 = 0xd6;
static const uint8 MSGPACK_FIXEXT8 = 0xd7;
static const uint8 MSGPACK_FIXEXT16 = 0xd8;
static const uint8 MSGPACK_STR8 = 0xd9;
static const uint8 MSGPACK_STR16 = 0xda;
static const uint8 MSGPACK_STR32 = 0xdb;
static const uint8 MSGPACK_ARRAY16 = 0xdc;
static const uint8 MSGPACK_ARRAY32 = 0xdd;
static const uint8 MSGPACK_MAP16 = 0xde;
static const uint8 MSGPACK_MAP32 = 0xdf;
static const uint8 MSGPACK_MINNEGATIVEFIXINT = 0xe0; // 224
static const uint8 MSGPACK_MAXNEGATIVEFIXINT = 0xff; // 255

FORCEINLINE EDcDataEntry TypeByteToDataEntry(uint8 TypeByte)
{
	using namespace  DcMsgPackCommon;

	switch (TypeByte)
	{
		case MSGPACK_NIL:
			return EDcDataEntry::None;
		case MSGPACK_TRUE:
		case MSGPACK_FALSE: 
			return EDcDataEntry::Bool;
		case MSGPACK_BIN8:
		case MSGPACK_BIN16:
		case MSGPACK_BIN32:
			return EDcDataEntry::Blob;
		case MSGPACK_FLOAT32:
			return EDcDataEntry::Float;
		case MSGPACK_FLOAT64:
			return EDcDataEntry::Double;
		case MSGPACK_INT8:
			return EDcDataEntry::Int8;
		case MSGPACK_INT16:
			return EDcDataEntry::Int16;
		case MSGPACK_INT32:
			return EDcDataEntry::Int32;
		case MSGPACK_INT64:
			return EDcDataEntry::Int64;
		case MSGPACK_UINT8:
			return EDcDataEntry::UInt8;
		case MSGPACK_UINT16:
			return EDcDataEntry::UInt16;
		case MSGPACK_UINT32:
			return EDcDataEntry::UInt32;
		case MSGPACK_UINT64:
			return EDcDataEntry::UInt64;
		case MSGPACK_STR8:
		case MSGPACK_STR16:
		case MSGPACK_STR32:
			return EDcDataEntry::String;
		case MSGPACK_ARRAY16:
		case MSGPACK_ARRAY32:
			return EDcDataEntry::ArrayRoot;
		case MSGPACK_MAP16:
		case MSGPACK_MAP32:
			return EDcDataEntry::MapRoot;
		case MSGPACK_FIXEXT1:
		case MSGPACK_FIXEXT2:
		case MSGPACK_FIXEXT4:
		case MSGPACK_FIXEXT8:
		case MSGPACK_FIXEXT16:
		case MSGPACK_EXT8:
		case MSGPACK_EXT16:
		case MSGPACK_EXT32:
			return EDcDataEntry::Extension;
		default:
			//	pass
			break;
	}

	if (TypeByte >= MSGPACK_MINFIXINT && TypeByte <= MSGPACK_MAXFIXINT)
		return EDcDataEntry::Int8;
	else if (TypeByte >= MSGPACK_MINNEGATIVEFIXINT && TypeByte <= MSGPACK_MAXNEGATIVEFIXINT)
		return EDcDataEntry::Int8;
	else if (TypeByte >= MSGPACK_MINFIXARRAY && TypeByte <= MSGPACK_MAXFIXARRAY)
		return EDcDataEntry::ArrayRoot;
	else if (TypeByte >= MSGPACK_MINFIXMAP && TypeByte <= MSGPACK_MAXFIXMAP)
		return EDcDataEntry::MapRoot;
	else if (TypeByte >= MSGPACK_MINFIXSTR && TypeByte <= MSGPACK_MAXFIXSTR)
		return EDcDataEntry::String;

	return EDcDataEntry::Ended;
}

static FORCEINLINE void SwapChar(uint8* Ptr, int FromIx, int ToIx)
{
	uint8 Tmp = Ptr[FromIx];
	Ptr[FromIx] = Ptr[ToIx]; 
	Ptr[ToIx] = Tmp;
}

template<int N>
FORCEINLINE void ReverseBytes(uint8* Ptr);

template<> FORCEINLINE void ReverseBytes<2>(uint8* Ptr)
{
	SwapChar(Ptr, 0, 1);
}

template<> FORCEINLINE void ReverseBytes<4>(uint8* Ptr)
{
	SwapChar(Ptr, 0, 3);
	SwapChar(Ptr, 1, 2);
}

template<> FORCEINLINE void ReverseBytes<8>(uint8* Ptr)
{
	SwapChar(Ptr, 0, 7);
	SwapChar(Ptr, 1, 6);
	SwapChar(Ptr, 2, 5);
	SwapChar(Ptr, 3, 4);
}

} // namespace DcMsgPackCommon

