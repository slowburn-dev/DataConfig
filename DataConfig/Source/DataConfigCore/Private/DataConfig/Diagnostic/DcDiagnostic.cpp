#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Diagnostic/DcDiagnosticAll.inl"
#include "DataConfig/Misc/DcTypeUtils.h"

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
		return FStringFormatArg(Var.GetValue<FString>().ReplaceCharWithEscapedChar());
	}
	else if (Var.DataType == EDcDataEntry::Text)
	{
		return FStringFormatArg(Var.GetValue<FText>().ToString().ReplaceCharWithEscapedChar());
	}
	else if (Var.DataType == EDcDataEntry::Name)
	{
		return FStringFormatArg(DcPropertyUtils::SafeNameToString(Var.GetValue<FName>()).ReplaceCharWithEscapedChar());
	}
	else if (Var.DataType == EDcDataEntry::Int8)
	{
		return FStringFormatArg(Var.GetValue<int8>());
	}
	else if (Var.DataType == EDcDataEntry::Int16)
	{
		return FStringFormatArg(Var.GetValue<int16>());
	}
	else if (Var.DataType == EDcDataEntry::Int32)
	{
		return FStringFormatArg(Var.GetValue<int32>());
	}
	else if (Var.DataType == EDcDataEntry::Int64)
	{
		return FStringFormatArg(Var.GetValue<int64>());
	}
	else if (Var.DataType == EDcDataEntry::UInt8)
	{
		return FStringFormatArg(Var.GetValue<uint8>());
	}
	else if (Var.DataType == EDcDataEntry::UInt16)
	{
		return FStringFormatArg(Var.GetValue<uint16>());
	}
	else if (Var.DataType == EDcDataEntry::UInt32)
	{
		return FStringFormatArg(Var.GetValue<uint32>());
	}
	else if (Var.DataType == EDcDataEntry::UInt64)
	{
		return FStringFormatArg(Var.GetValue<uint64>());
	}
	else if (Var.DataType == EDcDataEntry::Float)
	{
		return FStringFormatArg(Var.GetValue<float>());
	}
	else if (Var.DataType == EDcDataEntry::Double)
	{
		return FStringFormatArg(Var.GetValue<double>());
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

		UE_LOG(LogDataConfigCore, Display, TEXT("DataConfig Error: %s"), *FString::Format(Detail->Msg, FormatArgs));

		for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
		{
			if (Highlight.FileContext.IsSet())
			{
				UE_LOG(LogDataConfigCore, Display, TEXT("-->%s%d:%d"),
					*Highlight.FileContext->FilePath,
					Highlight.FileContext->Loc.Line,
					Highlight.FileContext->Loc.Column
				);
			}

			if (!Highlight.Formatted.IsEmpty())
			{
				UE_LOG(LogDataConfigCore, Display, TEXT("\n%s"),
					*Highlight.Formatted);
			}
		}
	}
	else
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Unknown Diagnostic ID: %d, %d"), Diag.Code.CategoryID, Diag.Code.ErrorID);
	}
}

