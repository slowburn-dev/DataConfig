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
	WriteStructValueFail,
	WriteStructAfterEnd,
	WriteStructEndFail,

	WriteClassFail,
	WriteClassKeyFail,
	WriteClassValueFail,
	WriteClassEndFail,
	WriteClassInlineNotCreated,

	WriteMapFail,
	WriteMapKeyFail,
	WriteMapValueFail,
	WriteMapEndFail,
	WriteMapAfterEnd,

	WriteArrayFail,
	WriteArrayAfterEnd,
	WriteArrayEndFail,

	WriteReferenceFail,

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
	ReadStructFail,
	ReadStructEndFail,
	ReadArrayFail,
	ReadArrayEndFail,
	ReadEndFail,

	ReadOutOfBoundMapKey,
	ReadOutOfBoundMapValue,

	//	Map
	ReadMapFail,
	ReadMapEndFail,
	ReadMapEndWhenStillHasValue,
	ReadMapAfterEnd,

	//	Class
	ReadClassFail,
	ReadClassKeyFail,
	ReadClassNextFail,
	ReadClassEndFail,

	StructKeyNotFound,
	ReadStructKeyFail,
	ReadStructNextFail,
	WriteStructKeyFail,
	StructEndWhenStillHasValue,
	ReadStructAfterEnded,

	//	Reference
	ReadReferenceFail,

	//	global fallback
	UnknownError,
};

} // namespace DataConfig

