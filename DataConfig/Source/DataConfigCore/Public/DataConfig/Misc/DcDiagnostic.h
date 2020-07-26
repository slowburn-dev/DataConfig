#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcDataVariant.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDiagnostic
{
	FErrorCode ID;
	TArray<FDataVariant> Args;

	FDiagnostic(FErrorCode InID) : ID(InID)
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

struct DATACONFIGCORE_API FDiagnosticDetail
{
	uint16 ID;
	TCHAR* Msg;
};

DATACONFIGCORE_API const FDiagnosticDetail* FindDiagnosticDetail(FErrorCode InError);

static const uint16 DETAIL_END = 0;

namespace DCommon
{

static const uint16 Category = 0x1;

enum Type : uint16
{
	Ok = 0,
	NotImplemented = 1,
};


}


} // namespace DataConfig



