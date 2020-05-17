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

struct FVisitor;

struct DATACONFIGCORE_API FMapAccess
{
	virtual ~FMapAccess();

	virtual FVisitResult Num(size_t& OutNum);
	virtual FVisitResult HasPending(bool& bOutHasPending);
	virtual FVisitResult ReadKey(FVisitor &Visitor);
	virtual FVisitResult ReadValue(FVisitor &Visitor);
	virtual FVisitResult Next();
};

struct DATACONFIGCORE_API FVisitor
{
	virtual ~FVisitor();

	virtual FVisitResult VisitBool(bool Value);
	virtual FVisitResult VisitName(FName Name);
	virtual FVisitResult VisitString(FString Str);
	virtual FVisitResult VisitMap(FMapAccess& MapAccess);

};



