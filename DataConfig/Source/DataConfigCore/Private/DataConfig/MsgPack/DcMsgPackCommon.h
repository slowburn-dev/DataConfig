#pragma once

#include "CoreTypes.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"

namespace DcMsgPackCommon
{

static const uint8 _MINFIXINT = 0x00; // 0
static const uint8 _MAXFIXINT = 0x7f; // 127
static const uint8 _MINFIXMAP = 0x80; // 128
static const uint8 _MAXFIXMAP = 0x8f; // 143
static const uint8 _MINFIXARRAY = 0x90; // 144
static const uint8 _MAXFIXARRAY = 0x9f; // 159
static const uint8 _MINFIXSTR = 0xa0; // 160
static const uint8 _MAXFIXSTR = 0xbf; // 191
static const uint8 _NIL = 0xc0;
static const uint8 _NEVERUSED = 0xc1;
static const uint8 _FALSE = 0xc2;
static const uint8 _TRUE = 0xc3;
static const uint8 _BIN8 = 0xc4;
static const uint8 _BIN16 = 0xc5;
static const uint8 _BIN32 = 0xc6;
static const uint8 _EXT8 = 0xc7;
static const uint8 _EXT16 = 0xc8;
static const uint8 _EXT32 = 0xc9;
static const uint8 _FLOAT32 = 0xca;
static const uint8 _FLOAT64 = 0xcb;
static const uint8 _UINT8 = 0xcc;
static const uint8 _UINT16 = 0xcd;
static const uint8 _UINT32 = 0xce;
static const uint8 _UINT64 = 0xcf;
static const uint8 _INT8 = 0xd0;
static const uint8 _INT16 = 0xd1;
static const uint8 _INT32 = 0xd2;
static const uint8 _INT64 = 0xd3;
static const uint8 _FIXEXT1 = 0xd4;
static const uint8 _FIXEXT2 = 0xd5;
static const uint8 _FIXEXT4 = 0xd6;
static const uint8 _FIXEXT8 = 0xd7;
static const uint8 _FIXEXT16 = 0xd8;
static const uint8 _STR8 = 0xd9;
static const uint8 _STR16 = 0xda;
static const uint8 _STR32 = 0xdb;
static const uint8 _ARRAY16 = 0xdc;
static const uint8 _ARRAY32 = 0xdd;
static const uint8 _MAP16 = 0xde;
static const uint8 _MAP32 = 0xdf;
static const uint8 _MINNEGATIVEFIXINT = 0xe0; // 224
static const uint8 _MAXNEGATIVEFIXINT = 0xff; // 255

FORCEINLINE EDcDataEntry TypeByteToDataEntry(uint8 TypeByte)
{
	using namespace  DcMsgPackCommon;

	switch (TypeByte)
	{
		case _NIL:
			return EDcDataEntry::Nil;
		case _TRUE:
		case _FALSE: 
			return EDcDataEntry::Bool;
		case _BIN8:
		case _BIN16:
		case _BIN32:
			return EDcDataEntry::Blob;
		case _FLOAT32:
			return EDcDataEntry::Float;
		case _FLOAT64:
			return EDcDataEntry::Double;
		case _INT8:
			return EDcDataEntry::Int8;
		case _INT16:
			return EDcDataEntry::Int16;
		case _INT32:
			return EDcDataEntry::Int32;
		case _INT64:
			return EDcDataEntry::Int64;
		case _UINT8:
			return EDcDataEntry::UInt8;
		case _UINT16:
			return EDcDataEntry::UInt16;
		case _UINT32:
			return EDcDataEntry::UInt32;
		case _UINT64:
			return EDcDataEntry::UInt64;
		case _STR8:
		case _STR16:
		case _STR32:
			return EDcDataEntry::String;
		case _ARRAY16:
		case _ARRAY32:
			return EDcDataEntry::ArrayRoot;
		case _MAP16:
		case _MAP32:
			return EDcDataEntry::MapRoot;
		case _FIXEXT1:
		case _FIXEXT2:
		case _FIXEXT4:
		case _FIXEXT8:
		case _FIXEXT16:
		case _EXT8:
		case _EXT16:
		case _EXT32:
			return EDcDataEntry::Extension;
		default:
			//	pass
			break;
	}

	if (TypeByte >= _MINFIXINT && TypeByte <= _MAXFIXINT)
		return EDcDataEntry::Int8;
	else if (TypeByte >= _MINNEGATIVEFIXINT && TypeByte <= _MAXNEGATIVEFIXINT)
		return EDcDataEntry::Int8;
	else if (TypeByte >= _MINFIXARRAY && TypeByte <= _MAXFIXARRAY)
		return EDcDataEntry::ArrayRoot;
	else if (TypeByte >= _MINFIXMAP && TypeByte <= _MAXFIXMAP)
		return EDcDataEntry::MapRoot;
	else if (TypeByte >= _MINFIXSTR && TypeByte <= _MAXFIXSTR)
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

