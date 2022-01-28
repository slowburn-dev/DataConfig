#include "DataConfig/MsgPack/DcMsgPackUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/MsgPack/DcMsgPackCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticMsgPack.h"
#include "UObject/TextProperty.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"

namespace DcMsgPackUtils
{
	
FName DC_META_MSGPACK_BLOB = FName(TEXT("DcMsgPackBlob"));

FDcResult MsgPackExtensionHandler(FDcReader* RawReader, FDcWriter* RawWriter)
{
	using namespace DcMsgPackCommon;

	FDcMsgPackReader* Reader;
	DC_TRY(DcCastReader(RawReader, Reader));
	FDcMsgPackWriter* Writer;
	DC_TRY(DcCastWriter(RawWriter, Writer));

	uint8 TypeByte;
	DC_TRY(Reader->PeekTypeByte(&TypeByte));

	switch (TypeByte)
	{
		case _FIXEXT1:
		{
			uint8 Type;
			uint8 Value;
			DC_TRY(Reader->ReadFixExt1(&Type, &Value));
			DC_TRY(Writer->WriteFixExt1(Type, Value));
			break;
		}
		case _FIXEXT2:
		{
			uint8 Type;
			FDcBytes2 Value;
			DC_TRY(Reader->ReadFixExt2(&Type, &Value));
			DC_TRY(Writer->WriteFixExt2(Type, Value));
			break;
		}
		case _FIXEXT4:
		{
			uint8 Type;
			FDcBytes4 Value;
			DC_TRY(Reader->ReadFixExt4(&Type, &Value));
			DC_TRY(Writer->WriteFixExt4(Type, Value));
			break;
		}
		case _FIXEXT8:
		{
			uint8 Type;
			FDcBytes8 Value;
			DC_TRY(Reader->ReadFixExt8(&Type, &Value));
			DC_TRY(Writer->WriteFixExt8(Type, Value));
			break;
		}
		case _FIXEXT16:
		{
			uint8 Type;
			FDcBytes16 Value;
			DC_TRY(Reader->ReadFixExt16(&Type, &Value));
			DC_TRY(Writer->WriteFixExt16(Type, Value));
			break;
		}
		case _EXT8:
		case _EXT16:
		case _EXT32:
		{
			uint8 Type;
			FDcBlobViewData Value;
			DC_TRY(Reader->ReadExt(&Type, &Value));
			DC_TRY(Writer->WriteExt(Type, Value));
			break;
		}
		default:
			return DC_FAIL(DcDMsgPack, TypeByteMismatchNoExpect) << TypeByte;
	}

	return DcOk();
}

FDcResult ReadExtBytes(FDcMsgPackReader* Reader, uint8& OutType, TArray<uint8>& OutBytes)
{
	using namespace DcMsgPackCommon;

	EDcDataEntry Next;
	DC_TRY(Reader->PeekRead(&Next));

	if (Next != EDcDataEntry::Extension)
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::Extension << Next;

	uint8 TypeByte;
	DC_TRY(Reader->PeekTypeByte(&TypeByte));

	switch (TypeByte)
	{
		case _FIXEXT1:
		{
			uint8 Value;
			DC_TRY(Reader->ReadFixExt1(&OutType, &Value));
			OutBytes.Add(Value);
			break;
		}
		case _FIXEXT2:
		{
			FDcBytes2 Value;
			DC_TRY(Reader->ReadFixExt2(&OutType, &Value));
			OutBytes.Append(Value.Data, 2);
			break;
		}
		case _FIXEXT4:
		{
			FDcBytes4 Value;
			DC_TRY(Reader->ReadFixExt4(&OutType, &Value));
			OutBytes.Append(Value.Data, 4);
			break;
		}
		case _FIXEXT8:
		{
			FDcBytes8 Value;
			DC_TRY(Reader->ReadFixExt8(&OutType, &Value));
			OutBytes.Append(Value.Data, 8);
			break;
		}
		case _FIXEXT16:
		{
			FDcBytes16 Value;
			DC_TRY(Reader->ReadFixExt16(&OutType, &Value));
			OutBytes.Append(Value.Data, 16);
			break;
		}
		case _EXT8:
		case _EXT16:
		case _EXT32:
		{
			FDcBlobViewData Value;
			DC_TRY(Reader->ReadExt(&OutType, &Value));
			OutBytes.Append(Value.DataPtr, Value.Num);
			break;
		}
		default:
			return DC_FAIL(DcDMsgPack, TypeByteMismatchNoExpect) << TypeByte;
	}

	return DcOk();
}

void VisitMsgPackPipeScalarPropertyClass(TFunctionRef<void(FFieldClass*)> Visitor)
{
	Visitor(FBoolProperty::StaticClass());

	Visitor(FInt8Property::StaticClass());
	Visitor(FInt16Property::StaticClass());
	Visitor(FIntProperty::StaticClass());
	Visitor(FInt64Property::StaticClass());

	Visitor(FByteProperty::StaticClass());
	Visitor(FUInt16Property::StaticClass());
	Visitor(FUInt32Property::StaticClass());
	Visitor(FUInt64Property::StaticClass());

	Visitor(FFloatProperty::StaticClass());
	Visitor(FDoubleProperty::StaticClass());

	Visitor(FStrProperty::StaticClass());
	Visitor(FNameProperty::StaticClass());
	Visitor(FTextProperty::StaticClass());
}

} // namespace DcMsgPackUtils

