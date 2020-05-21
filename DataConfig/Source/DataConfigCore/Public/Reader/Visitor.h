#pragma once

#include "CoreMinimal.h"

namespace DataConfig
{

enum class EReaderErrorCode : uint32;

struct DATACONFIGCORE_API FVisitResult
{
	uint32 Status : 31;
	uint32 bHasData : 1;

	FORCEINLINE bool Ok()
	{
		return Status == 0;
	}
};

static FORCEINLINE FVisitResult Ok() {
	return FVisitResult{0, 0};
}

static FORCEINLINE FVisitResult Fail(EReaderErrorCode Status) {
	check((uint32)Status != 0);
	return FVisitResult{ (uint32)Status, 0 };
}

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
	virtual FVisitResult VisitName(FName Value);
	virtual FVisitResult VisitString(FString Value);
	virtual FVisitResult VisitMap(FMapAccess& MapAccess);

};

} // namespace DataConfig


