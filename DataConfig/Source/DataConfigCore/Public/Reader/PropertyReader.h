#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Reader/Visitor.h"
#include "DataConfigTypes.h"

class UProperty;

namespace DataConfig
{

struct DATACONFIGCORE_API FPropertyReader  : private FNoncopyable
{
	FPropertyReader();
	~FPropertyReader();

	FPropertyReader(UObject* ClassObject);
	FPropertyReader(UObject* ClassObject, UProperty* Property);
	FPropertyReader(void* StructPtr, UScriptStruct* StructClass);
	FPropertyReader(void* StructPtr, UScriptStruct* StructClass, UProperty* Property);
	FPropertyReader(void* PrimitivePtr, UProperty* Property);

	FResult ReadAny(FVisitor &Visitor);

	FResult ReadBool(FVisitor &Visitor);
	FResult ReadName(FVisitor &Visitor);
	FResult ReadString(FVisitor &Visitor);

	FResult ReadFloat(FVisitor &Visitor);
	FResult ReadDouble(FVisitor &Visitor);

	FResult ReadInt8(FVisitor &Visitor);
	FResult ReadInt16(FVisitor &Visitor);
	FResult ReadInt(FVisitor &Visitor);
	FResult ReadInt64(FVisitor &Visitor);

	FResult ReadByte(FVisitor &Visitor);
	FResult ReadUInt16(FVisitor &Visitor);
	FResult ReadUInt32(FVisitor &Visitor);
	FResult ReadUInt64(FVisitor &Visitor);

	FResult ReadClass(FVisitor &Visitor);
	FResult ReadStruct(FVisitor &Visitor);
	FResult ReadMap(FVisitor &Visitor);

	struct FStructMapAccess;
	struct FClassMapAccess;

	//	TODO figure out if private even works
	//	TODO add a new TVaraint that works with this storage
	//		 ie the Variant don't contain storage it self

	//	TODO delete copy constructor, this isn't trival copyable
	using ImplStorageType = TAlignedStorage<56>::Type;
	ImplStorageType ImplStorage;
};

struct FPropertyReader::FStructMapAccess : public FMapAccess
{
	FPropertyReader& Parent;
	UProperty* Link;

	FStructMapAccess(FPropertyReader* Parent);

	FResult Num(TOptional<size_t>& OutNum) override;
	FResult HasPending(bool& bOutHasPending) override;
	FResult Next() override;
	FResult ReadKey(FVisitor &Visitor) override;
	FResult ReadValue(FVisitor &Visitor) override;
};

struct FPropertyReader::FClassMapAccess : public FMapAccess
{
	FPropertyReader& Parent;
	UProperty* Link;

	FClassMapAccess(FPropertyReader* Parent);

	FResult Num(TOptional<size_t>& OutNum) override;
	FResult HasPending(bool& bOutHasPending) override;
	FResult Next() override;
	FResult ReadKey(FVisitor &Visitor) override;
	FResult ReadValue(FVisitor &Visitor) override;
};


} // namespace DataConfig

