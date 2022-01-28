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

static_assert(DcIsPowerOf2(DcMsgPackUtils::FTypeByteQueue::_SIZE), "FTypeByteQueue::_SIZE needs to power of 2");

void RecordTypeByteOffset(DcMsgPackUtils::FTypeByteQueue& Self, int Offset);
int GetOldestOffset(DcMsgPackUtils::FTypeByteQueue& Self);
FString FormatMsgPackHighlight(FDcBlobViewData Blob, int End, const TCHAR* Header, const TCHAR* Tail);


} // namespace DcMsgPackCommon

