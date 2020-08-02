#include "DataConfig/DcTypes.h"
#include "DataConfig/DcErrorCodes.h"
#include "DataConfig/DcEnv.h"

DEFINE_LOG_CATEGORY(LogDataConfigCore);

namespace DataConfig {

EErrorCode GetReadErrorCode(EDataEntry DataEntry)
{
	switch (DataEntry)
	{
		//	Data Type
		case EDataEntry::Nil: return EErrorCode::ReadNilFail;
		case EDataEntry::Bool: return EErrorCode::ReadBoolFail;
		case EDataEntry::Name: return EErrorCode::ReadNameFail;
		case EDataEntry::String: return EErrorCode::ReadStringFail;
		case EDataEntry::Float: return EErrorCode::ReadFloatFail;
		case EDataEntry::Double: return EErrorCode::ReadDoubleFail;
		case EDataEntry::Int8: return EErrorCode::ReadInt8Fail;
		case EDataEntry::Int16: return EErrorCode::ReadInt16Fail;
		case EDataEntry::Int: return EErrorCode::ReadIntFail;
		case EDataEntry::Int64: return EErrorCode::ReadInt64Fail;
		case EDataEntry::Byte: return EErrorCode::ReadByteFail;
		case EDataEntry::UInt16: return EErrorCode::ReadUInt16Fail;
		case EDataEntry::UInt32: return EErrorCode::ReadUInt32Fail;
		case EDataEntry::UInt64: return EErrorCode::ReadUInt64Fail;
		case EDataEntry::StructRoot: return EErrorCode::ReadStructFail;
		case EDataEntry::StructEnd: return EErrorCode::ReadStructEndFail;
		case EDataEntry::ClassRoot: return EErrorCode::ReadClassFail;
		case EDataEntry::MapRoot: return EErrorCode::ReadMapFail;
		case EDataEntry::ArrayRoot: return EErrorCode::ReadArrayFail;
		case EDataEntry::Ended: return EErrorCode::ReadEndFail;
	}

	return EErrorCode::UnknownError;
}

EErrorCode GetWriteErrorCode(EDataEntry DataEntry)
{
	switch (DataEntry)
	{
		case EDataEntry::Nil: return EErrorCode::WriteNilFail;
		case EDataEntry::Bool: return EErrorCode::WriteBoolFail;
		case EDataEntry::Name: return EErrorCode::WriteNameFail;
		case EDataEntry::String: return EErrorCode::WriteStringFail;
		case EDataEntry::Float: return EErrorCode::WriteFloatFail;
		case EDataEntry::Double: return EErrorCode::WriteDoubleFail;
		case EDataEntry::Int8: return EErrorCode::WriteInt8Fail;
		case EDataEntry::Int16: return EErrorCode::WriteInt16Fail;
		case EDataEntry::Int: return EErrorCode::WriteIntFail;
		case EDataEntry::Int64: return EErrorCode::WriteInt64Fail;
		case EDataEntry::Byte: return EErrorCode::WriteByteFail;
		case EDataEntry::UInt16: return EErrorCode::WriteUInt16Fail;
		case EDataEntry::UInt32: return EErrorCode::WriteUInt32Fail;
		case EDataEntry::UInt64: return EErrorCode::WriteUInt64Fail;
		case EDataEntry::StructRoot: return EErrorCode::WriteStructRootFail;
		case EDataEntry::StructEnd: return EErrorCode::WriteStructEndFail;
		case EDataEntry::ClassRoot: return EErrorCode::WriteClassFail;
		case EDataEntry::MapRoot: return EErrorCode::WriteMapFail;
		case EDataEntry::ArrayRoot: return EErrorCode::WriteArrayFail;
		case EDataEntry::Ended: return EErrorCode::WriteEndFail;
	}

	return EErrorCode::UnknownError;
}

bool bInitialized = false;

void StartUp()
{
	bInitialized = true;
	PushEnv();
}

void ShutDown()
{
	PopEnv();
	bInitialized = false;
}

DATACONFIGCORE_API bool IsInitialized()
{
	return bInitialized;
}

} // namespace DataConfig
