#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcDataVariant.h"
#include "DataConfig/Source/DcSourceTypes.h"

#include "Containers/BasicArray.h"
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
	FString OwnerName;

	FString Formatted;
	TOptional<FDcDiagnosticFileContext> FileContext;

	FDcDiagnosticHighlight(void* InOwner, FString InOwnerName)
		: Owner(InOwner)
		, OwnerName(InOwnerName)
	{}
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

///	Use this to wrap around string to avoid escaping
struct FDcDiagnosticStringNoEscape
{
	FDcDiagnosticStringNoEscape(FString&& InStr)
		: Str(MoveTemp(InStr)) {}

	FString Str;
};

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FDcDiagnosticStringNoEscape&& NoEscapeStr)
{
	Diag.Args.Emplace(MoveTemp(NoEscapeStr.Str));
	return Diag;
};

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FString&& Str)
{
	Diag.Args.Emplace(Str.ReplaceCharWithEscapedChar());
	return Diag;
};

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, const FFieldVariant& Property)
{
	Diag.Args.Emplace(Property.GetFullName());
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

struct FDcStringDiagnosticConsumer : public IDcDiagnosticConsumer
{
	FORCEINLINE FDcStringDiagnosticConsumer(FString* InOutput)
		: Output(InOutput) {}

	void HandleDiagnostic(FDcDiagnostic& Diag) override;

	FString* Output;
};

struct DATACONFIGCORE_API FDcDiagnosticDetail
{
	uint16 ID;
	const TCHAR* Msg;
};

DATACONFIGCORE_API const FDcDiagnosticDetail* DcFindDiagnosticDetail(FDcErrorCode InError);
DATACONFIGCORE_API FStringFormatArg DcConvertArg(FDcDataVariant& Var);

struct DATACONFIGCORE_API FDcDiagnosticGroup
{
	uint16 CategoryID;
	size_t Count;
	FDcDiagnosticDetail* Details;
};

extern TBasicArray<FDcDiagnosticGroup*> DiagGroups;

DATACONFIGCORE_API void DcRegisterDiagnosticGroup(FDcDiagnosticGroup* InWeakGroup);

template <typename T, size_t N>
size_t DcDimOf(T(&)[N])
{
	return N;
}



