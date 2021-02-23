#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDEditorExtra
{

static FDcDiagnosticDetail _Details[] = {
	{ InvalidGameplayTagString, TEXT("Invalid Gameplay Tag String, Actual: '{0}'"), },
	{ InvalidGameplayTagStringFixErr, TEXT("Invalid Gameplay Tag String, Actual: '{0}', Fixed: '{1}', Error: '{2}'"), },
	{ InvalidGameplayAttribute, TEXT("Invalid GameplayAttribute String, Actual: '{0}"), },
	{ LoadObjectByPathFail, TEXT("'LoadObject()' by path failed, Expected Type: '{0}, Path: '{1}'"), },
	{ LoadFileByPathFail, TEXT("Load file failed, Path: '{0}'"), },
};

 FDcDiagnosticGroup Details = {
	DcDEditorExtra::Category,
	DcDimOf(_Details),
	_Details
};


} // namespace DcDEditorExtra
