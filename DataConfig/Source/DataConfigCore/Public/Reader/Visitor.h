#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"

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

	virtual FVisitResult Num(TOptional<size_t>& OutNum);
	virtual FVisitResult Next();
	virtual FVisitResult HasPending(bool& bOutHasPending);
	virtual FVisitResult ReadKey(FVisitor &Visitor);
	virtual FVisitResult ReadValue(FVisitor &Visitor);
};

struct DATACONFIGCORE_API FVisitor
{
	virtual ~FVisitor();

	virtual FVisitResult VisitBool(bool Value);
	virtual FVisitResult VisitName(const FName& Value);
	virtual FVisitResult VisitString(const FString& Value);

	virtual FVisitResult VisitFloat(float Value);
	virtual FVisitResult VisitDouble(double Value);

	virtual FVisitResult VisitInt8(int8 Value);
	virtual FVisitResult VisitInt16(int16 Value);
	virtual FVisitResult VisitInt(int Value);
	virtual FVisitResult VisitInt64(int64 Value);

	virtual FVisitResult VisitByte(uint8 Value);
	virtual FVisitResult VisitUInt16(uint16 Value);
	virtual FVisitResult VisitUInt32(uint32 Value);
	virtual FVisitResult VisitUInt64(uint64 Value);

	virtual FVisitResult VisitStruct(const FName& StructName, FMapAccess& MapAccess);
	virtual FVisitResult VisitClass(const FName& ClassName, FMapAccess& MapAccess);
	virtual FVisitResult VisitMap(FMapAccess& MapAccess);

};

} // namespace DataConfig


