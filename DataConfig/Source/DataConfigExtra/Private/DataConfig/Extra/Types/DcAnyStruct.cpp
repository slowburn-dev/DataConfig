#include "DataConfig/Extra/Types/DcAnyStruct.h"

void FDcAnyStruct::AnyStructReferenceController::DestroyObject()
{
	//	handle delete nullptr, which is valid
	if (DataPtr == nullptr)
	{
		check(StructClass == nullptr);
		return;
	}

	StructClass->DestroyStruct(DataPtr);
	FMemory::Free(DataPtr);
}
