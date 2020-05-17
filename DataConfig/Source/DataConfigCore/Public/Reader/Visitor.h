#pragma once

#include "CoreMinimal.h"

struct DATACONFIGCORE_API FVisitResult
{
	bool bOK = true;
	FString ErrMsg;

	static FORCEINLINE FVisitResult Ok() {
		return FVisitResult();
	}

	template<typename FmtType, typename... Types>
	static FVisitResult Fail(const FmtType& Fmt, Types... Args) {
		return FVisitResult{
			false,
			FString::Printf(Fmt, Args...),
		};
	}
};

struct DATACONFIGCORE_API FVisitor
{
	virtual ~FVisitor();

	virtual FVisitResult VisitBool(bool Value);
};



