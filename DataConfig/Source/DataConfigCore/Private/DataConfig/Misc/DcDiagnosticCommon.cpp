#include "DataConfig/Misc/DcDiagnostic.h"

namespace DataConfig
{

FDiagnosticDetail DCommonDetails[] = {
	{ DCommon::NotImplemented, TEXT("Calling into methods that are not implemented.") },
	{ DCommon::Fun, TEXT("Try Args {0} {1} {2}") },
	{ DETAIL_END },
};




} // namespace DataConfig
