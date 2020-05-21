#pragma once

namespace DataConfig
{

enum class EReaderErrorCode : uint32
{
	Ok = 0,
	//	Visitor
	UnexpectedBool,
	UnexpectedName,
	UnexpectedString,
	UnexpectedMap,
	UnexpectedMapNum,
	UnexpectedMapHasPending,
	UnexpectedMapKey,
	UnexpectedMapValue,
	UnexpectedMapNext,

	//	Reader
	DispatchAnyFail,
	ExpectBoolFail,
	ExpectNameFail,
	ExpectStringFail,
	ExpectStructFail,
	OutOfBoundMapKeyRead,
	OutOfBoundMapValueRead,


	//	Unknown
	UnknownError,

};

} // namespace DataConfig

