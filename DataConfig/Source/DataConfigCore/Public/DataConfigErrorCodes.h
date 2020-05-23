#pragma once

namespace DataConfig
{

enum class EErrorCode : uint32
{
	Ok = 0,

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

	//	Writer
	UnexpectedEnd,
	WriteAfterEnded,
	EndWhileWaitingKey,
	DupliatedEnd,

	//	Reader
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
	ReadOutOfBoundMapKey,
	ReadOutOfBoundMapValue,


	//	Map/Struct/Class
	StructKeyNotFound,
	StructExpectKeyFail,
	StructWriteNonKey,
	StructEndWhenStillHasValue,
	StructReadAfterEnded,

	//	Unknown
	UnknownError,
};

} // namespace DataConfig

