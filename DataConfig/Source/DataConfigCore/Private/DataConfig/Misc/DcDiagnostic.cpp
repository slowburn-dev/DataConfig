#include "DataConfig/Misc/DcDiagnostic.h"

namespace DataConfig
{

extern FDiagnosticDetail DCommonDetails[];

static const FDiagnosticDetail* SearchDetails(uint16 InID, const FDiagnosticDetail* Arr)
{
	while (true)
	{
		if (Arr->ID == DETAIL_END)
			break;

		if (Arr->ID == InID)
			return Arr;
	}

	return nullptr;
}

const FDiagnosticDetail* FindDiagnosticDetail(FErrorCode InError)
{
	if (InError.CategoryID == DCommon::Category)
		return SearchDetails(InError.ErrorID, DCommonDetails);

	return nullptr;
}

void FNullConsumer::HandleDiagnostic(FDiagnostic& Diag)
{
	// pass
	return;
}

} // namespace DataConfig
