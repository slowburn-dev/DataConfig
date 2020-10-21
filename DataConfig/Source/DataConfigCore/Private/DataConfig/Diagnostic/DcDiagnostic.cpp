#include "DataConfig/Diagnostic/DcDiagnostic.h"

#include "DataConfig/Diagnostic/DcDiagnosticAll.inl"

static const FDcDiagnosticDetail* SearchDetails(uint16 InID, const FDcDiagnosticGroup& Group)
{
	for (int Ix = 0; Ix < Group.Count; Ix++)
	{
		const FDcDiagnosticDetail& Detail = Group.Details[Ix];
		if (Detail.ID == InID)
		{
			return &Detail;
		}
	}

	return nullptr;
}

const FDcDiagnosticDetail* DcFindDiagnosticDetail(FDcErrorCode InError)
{
	//	TODO make this a jump table
	if (InError.CategoryID == DcDCommon::Category)
		return SearchDetails(InError.ErrorID, DCommonDetails);
	if (InError.CategoryID == DcDReadWrite::Category)
		return SearchDetails(InError.ErrorID, DPropertyReadWriteDetails);
	if (InError.CategoryID == DcDJSON::Category)
		return SearchDetails(InError.ErrorID, DJSONDetails);
	if (InError.CategoryID == DcDDeserialize::Category)
		return SearchDetails(InError.ErrorID, DDeserializeDetails);

	return nullptr;
}

void FDcNullDiagnosticConsumer::HandleDiagnostic(FDcDiagnostic& Diag)
{
	return;
}

FDcDefaultLogDiagnosticConsumer::FDcDefaultLogDiagnosticConsumer()
	: Override(TEXT("LogDataConfigCore"), ELogVerbosity::Display)
{
}

FStringFormatArg DcConvertArg(FDcDataVariant& Var)
{
	if (Var.DataType == EDcDataEntry::Bool)
	{
		return FStringFormatArg(Var.GetValue<bool>());
	}
	else if (Var.DataType == EDcDataEntry::Nil)
	{
		return FStringFormatArg(TEXT("<null>"));
	}
	else if (Var.DataType == EDcDataEntry::String)
	{
		return FStringFormatArg(Var.GetValue<FString>());
	}
	else if (Var.DataType == EDcDataEntry::Name)
	{
		return FStringFormatArg(Var.GetValue<FName>().ToString());
	}
	else if (Var.DataType == EDcDataEntry::Int)
	{
		return FStringFormatArg(Var.GetValue<int32>());
	}
	else
	{
		return FStringFormatArg(TEXT("<unsupported type>"));
	}
}


void FDcDefaultLogDiagnosticConsumer::HandleDiagnostic(FDcDiagnostic& Diag)
{
	const FDcDiagnosticDetail* Detail = DcFindDiagnosticDetail(Diag.Code);
	if (Detail)
	{
		check(Detail->ID == Diag.Code.ErrorID);
		TArray<FStringFormatArg> FormatArgs;
		for (FDcDataVariant& Var : Diag.Args)
			FormatArgs.Add(DcConvertArg(Var));
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *FString::Format(Detail->Msg, FormatArgs));
	}
	else
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Unknown Diagnostic ID: %d, %d"), Diag.Code.CategoryID, Diag.Code.ErrorID);
	}
}

