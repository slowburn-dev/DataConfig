#pragma once

#include "DataConfig/DcTypes.h"

struct FDcReader;
struct FDcWriter;
struct FDcMsgPackReader;

namespace DcMsgPackUtils
{

DATACONFIGCORE_API extern const FName DC_META_MSGPACK_BLOB;

DATACONFIGCORE_API FDcResult MsgPackExtensionHandler(FDcReader* Reader, FDcWriter* Writer);

DATACONFIGCORE_API FDcResult ReadExtBytes(FDcMsgPackReader* Reader, uint8& OutType, TArray<uint8>& OutBytes);

} // namespace DcMsgPackUtils



