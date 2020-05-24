#pragma once

namespace DataConfig
{

enum class EErrorCode : uint32
{
	Ok = 0,

	WriteNilFail,
	WriteBoolFail,
	WriteNameFail,
	WriteStringFail,

	WriteFloatFail,
	WriteDoubleFail,

	WriteInt8Fail,
	WriteInt16Fail,
	WriteIntFail,
	WriteInt64Fail,

	WriteByteFail,
	WriteUInt16Fail,
	WriteUInt32Fail,
	WriteUInt64Fail,

	WriteStructRootFail,
	WriteStructEndFail,
	WriteClassFail,

	WriteMapFail,
	WriteArrayFail,

	//	Writer
	WriteEndFail,
	WriteAfterEnded,
	EndWhileWaitingKey,
	DupliatedEnd,

	//	Reader
	ReadNilFail,
	ReadBoolFail,
	ReadNameFail,

	ReadFloatFail,
	ReadDoubleFail,

	ReadInt8Fail,
	ReadInt16Fail,
	ReadIntFail,
	ReadInt64Fail,

	ReadByteFail,
	ReadUInt16Fail,
	ReadUInt32Fail,
	ReadUInt64Fail,

	ReadStringFail,
	ReadClassFail,
	ReadStructFail,
	ReadStructEndFail,
	ReadMapFail,
	ReadArrayFail,
	ReadEndFail,

	ReadOutOfBoundMapKey,
	ReadOutOfBoundMapValue,


	//	Map/Struct/Class
	StructKeyNotFound,
	ReadStructKeyFail,
	WriteStructKeyFail,
	StructEndWhenStillHasValue,
	StructReadAfterEnded,

	//	Unknown
	UnknownError,
};

} // namespace DataConfig

