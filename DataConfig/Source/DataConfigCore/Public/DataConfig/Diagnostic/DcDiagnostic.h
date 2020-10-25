#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcDataVariant.h"

#include "Templates/IsEnumClass.h"

struct DATACONFIGCORE_API FDcInputSpan
{
	uint32 Line;
	uint32 ColBegin;
	uint32 SpanLen;

	FString Formatted;
};

struct DATACONFIGCORE_API FDcFileSpan 
{
	FDcInputSpan Span;
	FString FilePath;

	FString Formatted;

	FORCEINLINE bool IsValid()
	{
		return !Formatted.IsEmpty();
	}
};

struct DATACONFIGCORE_API FDcDiagnostic
{
	FDcErrorCode Code;
	TArray<FDcDataVariant> Args;

	TOptional<FDcFileSpan> Span;

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

//	TODO UENUM -> FName conversion
template<typename T>
FORCEINLINE typename TEnableIf<TIsEnumClass<T>::Value, FDcDiagnostic&>::Type
operator<<(FDcDiagnostic& Diag, T&& InValue)
{
	Diag.Args.Emplace((int)(InValue));
	return Diag;
}

FORCEINLINE FDcDiagnostic& operator<<(FDcDiagnostic& Diag, EDcDataEntry Entry)
{
	Diag.Args.Emplace(FName(TEXT("<DataEntry>")));
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



