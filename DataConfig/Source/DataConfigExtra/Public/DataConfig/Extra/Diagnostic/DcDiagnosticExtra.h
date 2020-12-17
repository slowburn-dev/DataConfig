#pragma once

#include "HAL/Platform.h"

///	Extra diagnostic 

namespace DcDExtra
{

static const uint16 Category = 0xF001;

enum Type : uint16
{
	Unknown = 0,
	InvalidBase64String,
};

extern DATACONFIGEXTRA_API FDcDiagnosticGroup Details;

} // namespace DcDExtra

