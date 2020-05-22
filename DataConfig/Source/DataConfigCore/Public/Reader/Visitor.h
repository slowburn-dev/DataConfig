#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "DataConfigTypes.h"

namespace DataConfig
{

struct FVisitor;

struct DATACONFIGCORE_API FMapAccess
{
	virtual ~FMapAccess();

	virtual FResult Num(TOptional<size_t>& OutNum);
	virtual FResult Next();
	virtual FResult HasPending(bool& bOutHasPending);
	virtual FResult ReadKey(FVisitor &Visitor);
	virtual FResult ReadValue(FVisitor &Visitor);
};

struct DATACONFIGCORE_API FVisitor
{
	virtual ~FVisitor();

	virtual FResult VisitBool(bool Value);
	virtual FResult VisitName(const FName& Value);
	virtual FResult VisitString(const FString& Value);

	virtual FResult VisitFloat(float Value);
	virtual FResult VisitDouble(double Value);

	virtual FResult VisitInt8(int8 Value);
	virtual FResult VisitInt16(int16 Value);
	virtual FResult VisitInt(int Value);
	virtual FResult VisitInt64(int64 Value);

	virtual FResult VisitByte(uint8 Value);
	virtual FResult VisitUInt16(uint16 Value);
	virtual FResult VisitUInt32(uint32 Value);
	virtual FResult VisitUInt64(uint64 Value);

	virtual FResult VisitStruct(const FName& StructName, FMapAccess& MapAccess);
	virtual FResult VisitClass(const FName& ClassName, FMapAccess& MapAccess);
	virtual FResult VisitMap(FMapAccess& MapAccess);

};

} // namespace DataConfig


