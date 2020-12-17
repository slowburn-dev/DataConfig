#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcDataVariant.h"
#include "DataConfig/Source/DcSourceTypes.h"

#include "Templates/IsEnumClass.h"
#include "UObject/Package.h"

struct DATACONFIGCORE_API FDcDiagnosticFileContext
{
	FDcSourceLocation Loc;
	FString FilePath;
};

struct DATACONFIGCORE_API FDcDiagnosticHighlight
{
	void* Owner;
	FString Formatted;
	TOptional<FDcDiagnosticFileContext> FileContext;

	FDcDiagnosticHighlight(void* InOwner) : Owner(InOwner) {}
};

struct DATACONFIGCORE_API FDcDiagnostic
{
	FDcErrorCode Code;
	TArray<FDcDataVariant> Args;

	TArray<FDcDiagnosticHighlight> Highlights;

	FDcDiagnostic(FDcErrorCode InID) : Code(InID)
	{}

	operator FDcResult() const {
		return FDcResult{ FDcResult::EStatus::Error };
	}
};

template<typename T>
FORCEINLINE typename TEnableIf<!TIsEnumClass<T>::Value, FDcDiagnostic&>::Type
operator<<(FDcDiagnostic& Diag, T&& InValue)
{
	Diag.Args.Emplace(Forward<T>(InValue));
	return Diag;
}

template<typename T>
FORCEINLINE typename TEnableIf<TIsEnumClass<T>::Value, FDcDiagnostic&>::Type
operator<<(FDcDiagnostic& Diag, T&& InValue)
{
	Diag.Args.Emplace((int)(InValue));
	return Diag;
}

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, ANSICHAR Char)
{
	if (FCharAnsi::IsPrint(Char))
	{
		Diag.Args.Emplace(FString::Chr((TCHAR)Char));
	}
	else
	{
		FString Hex(TEXT("0x"));
		ByteToHex((uint8)Char, Hex);
		Diag.Args.Emplace(Hex);
	}

	return Diag;
}

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, WIDECHAR Char)
{
	if (FCharWide::IsPrint(Char))
	{
		Diag.Args.Emplace(FString::Chr(Char));
	}
	else
	{
		const char* Ch1 = (const char*)&Char;
		const char* Ch2 = Ch1 + 1;

		FString Hex;
		ByteToHex((uint8)*Ch1, Hex);
		ByteToHex((uint8)*Ch2, Hex);
		Diag.Args.Emplace(Hex);
	}

	return Diag;
}

FORCEINLINE_DEBUGGABLE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, EDcDataEntry Entry)
{
	UEnum* DataEntryEnum = ::FindObject<UEnum>(ANY_PACKAGE, TEXT("EDcDataEntry"), true);
	check(DataEntryEnum);
	Diag.Args.Emplace(DataEntryEnum->GetNameByIndex((int32)Entry));
	return Diag;
}

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FDcDiagnosticHighlight&& DiagSpan)
{
	Diag.Highlights.Emplace(MoveTemp(DiagSpan));
	return Diag;
}

struct IDcDiagnosticConsumer : public TSharedFromThis<IDcDiagnosticConsumer>
{
	virtual void HandleDiagnostic(FDcDiagnostic& Diag) = 0;
	virtual ~IDcDiagnosticConsumer() = default;
};

struct FDcNullDiagnosticConsumer : public IDcDiagnosticConsumer
{
	void HandleDiagnostic(FDcDiagnostic& Diag) override;
};

struct FDcDefaultLogDiagnosticConsumer : public IDcDiagnosticConsumer
{
	FDcDefaultLogDiagnosticConsumer();

	void HandleDiagnostic(FDcDiagnostic& Diag) override;

	FLogScopedCategoryAndVerbosityOverride Override;
};

struct DATACONFIGCORE_API FDcDiagnosticDetail
{
	uint16 ID;
	TCHAR* Msg;
};

DATACONFIGCORE_API const FDcDiagnosticDetail* DcFindDiagnosticDetail(FDcErrorCode InError);
DATACONFIGCORE_API FStringFormatArg DcConvertArg(FDcDataVariant& Var);

static const uint16 DC_DETAIL_END = MAX_uint16;

struct DATACONFIGCORE_API FDcDiagnosticGroup
{
	size_t Count;
	FDcDiagnosticDetail* Details;
};

template <typename T, size_t N>
size_t DcDimOf(T(&)[N])
{
	return N;
}



