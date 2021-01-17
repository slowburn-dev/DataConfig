#pragma once

#include "HAL/Platform.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

///	Editor Extra diagnostic 

namespace DcDEditorExtra
{

static const uint16 Category = 0xF002;

enum Type : uint16
{
	Unknown = 0,
	//	Gameplay Tag
	InvalidGameplayTagString,
	InvalidGameplayTagStringFixErr,
	//	BP Class
	LoadObjectByPathFail,
};

extern DATACONFIGEDITOREXTRA_API FDcDiagnosticGroup Details;

} // namespace DcDEditorExtra

