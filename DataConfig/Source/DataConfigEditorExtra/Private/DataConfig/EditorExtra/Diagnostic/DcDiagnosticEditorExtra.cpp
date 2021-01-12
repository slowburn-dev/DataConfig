#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDEditorExtra
{

static FDcDiagnosticDetail _Details[] = {
	{ InvalidGameplayTagString, TEXT("Invalid Gameplay Tag String, Actual: '{0}'"), },
	{ InvalidGameplayTagStringFixErr, TEXT("Invalid Gameplay Tag String, Actual: '{0}', Fixed: '{1}', Error: '{2}'"), },
};

 FDcDiagnosticGroup Details = {
	DcDEditorExtra::Category,
	DcDimOf(_Details),
	_Details
};


} // namespace DcDEditorExtra
