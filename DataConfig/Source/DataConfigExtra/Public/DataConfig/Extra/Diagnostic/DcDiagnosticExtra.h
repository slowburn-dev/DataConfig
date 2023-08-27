#pragma once

#include "HAL/Platform.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

///	Extra diagnostic 

namespace DcDExtra
{

static const uint16 Category = 0xF001;

enum Type : uint16
{
	Unknown = 0,
	InvalidBase64String,
	ExpectClassExpand,

	//	Sqlite
	SqliteLastError,
	SqliteBusy,

	//	InlineStruct
	InlineStructTooBig,
	InlineStructNotSet,

	//	Nested
	NestedMissingMetaData,
	NestedGrid2DHeightMismatch,
	NestedGrid2DWidthMismatch,
	NestedGrid2DLenMismatch,

};

extern DATACONFIGEXTRA_API FDcDiagnosticGroup Details;

} // namespace DcDExtra

