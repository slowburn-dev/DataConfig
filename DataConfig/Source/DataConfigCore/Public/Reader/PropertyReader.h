#pragma once

#include "CoreMinimal.h"
#include "Reader/Visitor.h"
#include "DataConfigTypes.h"

struct DATACONFIGCORE_API FPropertyReader 
{
	FVisitResult ReadBool(FVisitor &Visitor);

	FPropertyReader();
	~FPropertyReader();

	FPropertyReader(UObject* ClassObject);
	FPropertyReader(UObject* ClassObject, UProperty* Property);
	FPropertyReader(void* StructPtr, UScriptStruct* StructClass);
	FPropertyReader(void* StructPtr, UScriptStruct* StructClass, UProperty* Property);
	FPropertyReader(void* PrimitivePtr, UProperty* Property);

	/*	TODO how to get this private working?
private:
	friend struct ReaderState& GetState(FPropertyReader& Self);
	*/

	//	TODO add a new TVaraint that works with this storage
	//		 ie the Variant don't contain storage it self
	using ImplStorageType = AlignedStorage<128>::Type;
	ImplStorageType ImplStorage;
};





