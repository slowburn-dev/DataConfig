#pragma once

namespace DataConfig
{

enum class EErrorCode : uint32
{
	Ok = 0,
	//	Visitor
	UnexpectedBool,
	UnexpectedName,
	UnexpectedString,

	UnexpectedFloat,
	UnexpectedDouble,

	UnexpectedInt8,
	UnexpectedInt16,
	UnexpectedInt,
	UnexpectedInt64,

	UnexpectedByte,
	UnexpectedUInt16,
	UnexpectedUInt32,
	UnexpectedUInt64,

	UnexpectedStruct,
	UnexpectedClass,

	UnexpectedMap,
	UnexpectedMapAccess,

	//	Reader
	DispatchAnyFail,
	ExpectBoolFail,
	ExpectNameFail,

	ExpectFloatFail,
	ExpectDoubleFail,

	ExpectInt8Fail,
	ExpectInt16Fail,
	ExpectIntFail,
	ExpectInt64Fail,

	ExpectByteFail,
	ExpectUInt16Fail,
	ExpectUInt32Fail,
	ExpectUInt64Fail,

	ExpectStringFail,
	ExpectClassFail,
	ExpectStructFail,
	ExpectStructEndFail,
	ExpectMapFail,
	OutOfBoundMapKeyRead,
	OutOfBoundMapValueRead,

	//	Writer
	UnexpectedEnd,
	WriteAfterEnded,
	EndWhileWaitingKey,
	DupliatedEnd,

	//	Map/Struct/Class
	StructKeyNotFound,
	StructWriteNonKey,
	StructEndWhenStillHasValue,

	//	Unknown
	UnknownError,

};

} // namespace DataConfig

