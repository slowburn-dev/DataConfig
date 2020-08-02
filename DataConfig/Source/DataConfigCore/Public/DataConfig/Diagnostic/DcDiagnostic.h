#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcDataVariant.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDiagnostic
{
	FErrorCode Code;
	TArray<FDataVariant> Args;

	FDiagnostic(FErrorCode InID) : Code(InID)
	{}

	operator FResult() const {
		return Fail();
	}
};

template<typename T>
FORCEINLINE FDiagnostic& operator<<(FDiagnostic& Diag, T&& InValue)
{
	Diag.Args.Emplace(Forward<T>(InValue));
	return Diag;
}

struct IDiagnosticConsumer : public TSharedFromThis<IDiagnosticConsumer>
{
	virtual void HandleDiagnostic(FDiagnostic& Diag) = 0;
	virtual ~IDiagnosticConsumer() = default;
};

struct FNullDiagnosticConsumer : public IDiagnosticConsumer
{
	void HandleDiagnostic(FDiagnostic& Diag) override;
};

struct FDefaultLogDiagnosticConsumer : public IDiagnosticConsumer
{
	FDefaultLogDiagnosticConsumer();

	void HandleDiagnostic(FDiagnostic& Diag) override;

	FLogScopedCategoryAndVerbosityOverride Override;
};

struct DATACONFIGCORE_API FDiagnosticDetail
{
	uint16 ID;
	TCHAR* Msg;
};

DATACONFIGCORE_API const FDiagnosticDetail* FindDiagnosticDetail(FErrorCode InError);

static const uint16 DETAIL_END = MAX_uint16;

struct DATACONFIGCORE_API FDiagnosticGroup
{
	size_t Count;
	FDiagnosticDetail* Details;
};

template <typename T, size_t N>
size_t DimOf(T(&)[N])
{
	return N;
}

} // namespace DataConfig


