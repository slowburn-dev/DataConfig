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

void FNullDiagnosticConsumer::HandleDiagnostic(FDiagnostic& Diag)
{
	return;
}

FDefaultLogDiagnosticConsumer::FDefaultLogDiagnosticConsumer()
	: Override(TEXT("LogDataConfigCore"), ELogVerbosity::Display)
{
}

static FStringFormatArg ConvertArg(FDataVariant& Var)
{
	if (Var.DataType == EDataEntry::Bool)
	{
		return FStringFormatArg(Var.GetValue<bool>());
	}
	else if (Var.DataType == EDataEntry::Nil)
	{
		return FStringFormatArg(TEXT("<null>"));
	}
	else if (Var.DataType == EDataEntry::Nil)
	{
		return FStringFormatArg(Var.GetValue<FString>());
	}
	else
	{
		return FStringFormatArg(TEXT("<unsupported type>"));
	}
}


void FDefaultLogDiagnosticConsumer::HandleDiagnostic(FDiagnostic& Diag)
{
	const FDiagnosticDetail* Detail = FindDiagnosticDetail(Diag.Code);
	if (Detail)
	{
		check(Detail->ID == Diag.Code.ErrorID);
		TArray<FStringFormatArg> FormatArgs;
		for (FDataVariant& Var : Diag.Args)
			FormatArgs.Add(ConvertArg(Var));
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *FString::Format(Detail->Msg, FormatArgs));
	}
	else
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Unknown Diagnostic ID: %d, %d"), Diag.Code.CategoryID, Diag.Code.ErrorID);
	}
}

} // namespace DataConfig
