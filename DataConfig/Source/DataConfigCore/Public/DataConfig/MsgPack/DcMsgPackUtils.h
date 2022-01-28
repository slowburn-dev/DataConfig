#pragma once

#include "DataConfig/DcTypes.h"

struct FDcReader;
struct FDcWriter;
struct FDcMsgPackReader;

namespace DcMsgPackUtils
{

extern FName DC_META_MSGPACK_BLOB;

DATACONFIGCORE_API FDcResult MsgPackExtensionHandler(FDcReader* Reader, FDcWriter* Writer);

DATACONFIGCORE_API FDcResult ReadExtBytes(FDcMsgPackReader* Reader, uint8& OutType, TArray<uint8>& OutBytes);

DATACONFIGCORE_API void VisitMsgPackPipeScalarPropertyClass(TFunctionRef<void(FFieldClass*)> Visitor);

struct FTypeByteQueue
{
	static constexpr int _SIZE = 8;

	uint8 LastTypeByteHead;
	enum class EState : uint8
	{
		Empty,
		NoFlippedYet,
		Flipped,
	};
	EState State;
	int LastTypeBytes[_SIZE];
};


} // namespace DcMsgPackUtils



