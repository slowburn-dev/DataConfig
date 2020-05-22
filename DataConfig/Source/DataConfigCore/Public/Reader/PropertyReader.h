#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Reader/Visitor.h"
#include "DataConfigTypes.h"

class UProperty;

namespace DataConfig
{


struct DATACONFIGCORE_API FPropertyReader 
{
	FPropertyReader();
	~FPropertyReader();

	FPropertyReader(UObject* ClassObject);
	FPropertyReader(UObject* ClassObject, UProperty* Property);
	FPropertyReader(void* StructPtr, UScriptStruct* StructClass);
	FPropertyReader(void* StructPtr, UScriptStruct* StructClass, UProperty* Property);
	FPropertyReader(void* PrimitivePtr, UProperty* Property);

	FVisitResult ReadAny(FVisitor &Visitor);

	FVisitResult ReadBool(FVisitor &Visitor);
	FVisitResult ReadName(FVisitor &Visitor);
	FVisitResult ReadString(FVisitor &Visitor);

	FVisitResult ReadFloat(FVisitor &Visitor);
	FVisitResult ReadDouble(FVisitor &Visitor);

	FVisitResult ReadInt8(FVisitor &Visitor);
	FVisitResult ReadInt16(FVisitor &Visitor);
	FVisitResult ReadInt(FVisitor &Visitor);
	FVisitResult ReadInt64(FVisitor &Visitor);

	FVisitResult ReadByte(FVisitor &Visitor);
	FVisitResult ReadUInt16(FVisitor &Visitor);
	FVisitResult ReadUInt32(FVisitor &Visitor);
	FVisitResult ReadUInt64(FVisitor &Visitor);

	FVisitResult ReadClass(FVisitor &Visitor);
	FVisitResult ReadStruct(FVisitor &Visitor);
	FVisitResult ReadMap(FVisitor &Visitor);

	struct FStructMapAccess;
	struct FClassMapAccess;

	//	TODO figure out if private even works
	//	TODO add a new TVaraint that works with this storage
	//		 ie the Variant don't contain storage it self

	//	TODO delete copy constructor, this isn't trival copyable
	using ImplStorageType = TAlignedStorage<64>::Type;
	ImplStorageType ImplStorage;
};

struct FPropertyReader::FStructMapAccess : public FMapAccess
{
	FPropertyReader& Parent;
	UProperty* Link;

	FStructMapAccess(FPropertyReader* Parent);

	FVisitResult Num(TOptional<size_t>& OutNum) override;
	FVisitResult HasPending(bool& bOutHasPending) override;
	FVisitResult Next() override;
	FVisitResult ReadKey(FVisitor &Visitor) override;
	FVisitResult ReadValue(FVisitor &Visitor) override;
};

struct FPropertyReader::FClassMapAccess : public FMapAccess
{
	FPropertyReader& Parent;
	UProperty* Link;

	FClassMapAccess(FPropertyReader* Parent);

	FVisitResult Num(TOptional<size_t>& OutNum) override;
	FVisitResult HasPending(bool& bOutHasPending) override;
	FVisitResult Next() override;
	FVisitResult ReadKey(FVisitor &Visitor) override;
	FVisitResult ReadValue(FVisitor &Visitor) override;
};


} // namespace DataConfig

