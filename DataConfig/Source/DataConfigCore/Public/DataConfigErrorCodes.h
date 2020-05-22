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
	ExpectMapFail,
	OutOfBoundMapKeyRead,
	OutOfBoundMapValueRead,

	//	Writer
	UnexpectedEnd,
	WriteAfterEnded,

	//	Unknown
	UnknownError,

};

} // namespace DataConfig

