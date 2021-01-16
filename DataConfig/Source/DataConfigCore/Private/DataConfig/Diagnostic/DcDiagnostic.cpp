#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/DcEnv.h"

TBasicArray<FDcDiagnosticGroup*> DiagGroups;

void DcRegisterDiagnosticGroup(FDcDiagnosticGroup* InWeakGroup)
{
	checkf(!DcIsInitialized(), TEXT("can't register diagnostic group after initialized"));
	checkf(InWeakGroup->CategoryID > 0xFF, TEXT("categoryID under 255 is reserved"));
	DiagGroups.Emplace(InWeakGroup);
}

static FDcDiagnosticDetail* SearchDetails(uint16 InID, FDcDiagnosticGroup* Group)
{
	for (size_t Ix = 0; Ix < Group->Count; Ix++)
	{
		FDcDiagnosticDetail& Detail = Group->Details[Ix];
		if (Detail.ID == InID)
		{
			return &Detail;
		}
	}

	return nullptr;
}

const FDcDiagnosticDetail* DcFindDiagnosticDetail(FDcErrorCode InError)
{
	for (FDcDiagnosticGroup* Group : DiagGroups)
	{
		if (InError.CategoryID == Group->CategoryID)
			return SearchDetails(InError.ErrorID, Group);
	}

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
		//	FString escaping is handled on push
		return FStringFormatArg(Var.GetValue<FString>());
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

		UE_LOG(LogDataConfigCore, Display, TEXT("# DataConfig Error: %s"), *FString::Format(Detail->Msg, FormatArgs));

		for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
		{
			if (Highlight.FileContext.IsSet())
			{
				UE_LOG(LogDataConfigCore, Display, TEXT("- [%s] --> %s%d:%d\n%s"),
					*Highlight.OwnerName,
					*Highlight.FileContext->FilePath,
					Highlight.FileContext->Loc.Line,
					Highlight.FileContext->Loc.Column,
					*Highlight.Formatted
				);
			}
			else
			{
				UE_LOG(LogDataConfigCore, Display, TEXT("- [%s] %s"),
					*Highlight.OwnerName,
					*Highlight.Formatted);
			}
		}
	}
	else
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Unknown Diagnostic ID: %d, %d"), Diag.Code.CategoryID, Diag.Code.ErrorID);
	}
}

void FDcStringDiagnosticConsumer::HandleDiagnostic(FDcDiagnostic& Diag)
{
	check(Output);
	const FDcDiagnosticDetail* Detail = DcFindDiagnosticDetail(Diag.Code);
	if (Detail)
	{
		check(Detail->ID == Diag.Code.ErrorID);
		TArray<FStringFormatArg> FormatArgs;
		for (FDcDataVariant& Var : Diag.Args)
			FormatArgs.Add(DcConvertArg(Var));

		Output->Appendf(TEXT("# DataConfig Error: %s"), *FString::Format(Detail->Msg, FormatArgs));
		Output->AppendChar(TCHAR('\n'));

		for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
		{
			if (Highlight.FileContext.IsSet())
			{
				Output->Appendf(TEXT("- [%s] --> %s%d:%d\n%s"),
					*Highlight.OwnerName,
					*Highlight.FileContext->FilePath,
					Highlight.FileContext->Loc.Line,
					Highlight.FileContext->Loc.Column,
					*Highlight.Formatted
				);
			}
			else
			{
				Output->Appendf(TEXT("- [%s] %s"),
					*Highlight.OwnerName,
					*Highlight.Formatted);
			}
			Output->AppendChar(TCHAR('\n'));
		}
	}
	else
	{
		Output->Appendf(TEXT("Unknown Diagnostic ID: %d, %d"), Diag.Code.CategoryID, Diag.Code.ErrorID);
		Output->AppendChar(TCHAR('\n'));
	}
}
