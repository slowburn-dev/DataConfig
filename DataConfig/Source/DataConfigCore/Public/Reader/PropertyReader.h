#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Reader/Visitor.h"
#include "DataConfigTypes.h"

class UProperty;

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

	FVisitResult ReadStruct(FVisitor &Visitor);

	struct FStructMapAccess : public FMapAccess
	{
		FPropertyReader& Parent;
		TWeakObjectPtr<UProperty> Link;

		FStructMapAccess(FPropertyReader* Parent);

		FVisitResult Num(size_t& OutNum) override;
		FVisitResult HasPending(bool& bOutHasPending) override;
		FVisitResult Next() override;
		FVisitResult ReadKey(FVisitor &Visitor) override;
		FVisitResult ReadValue(FVisitor &Visitor) override;
	};

	//	TODO figure out if private even works
	//	TODO add a new TVaraint that works with this storage
	//		 ie the Variant don't contain storage it self
	using ImplStorageType = AlignedStorage<64>::Type;
	ImplStorageType ImplStorage;
};





