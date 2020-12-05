#include "DataConfig/Extra/Types/DcAnyStruct.h"

void FDcAnyStruct::AnyStructReferenceController::DestroyObject()
{
	check(AnyStruct);

	//	handle delete nullptr, which is valid
	if (AnyStruct->DataPtr == nullptr)
	{
		check(AnyStruct->StructClass == nullptr);
		return;
	}

	AnyStruct->StructClass->DestroyStruct(AnyStruct->DataPtr);
	FMemory::Free(AnyStruct->DataPtr);
}

TCHAR* FDcAnyStruct::GetTypeNameLiteral()
{
	if (!StructClass)
		return nullptr;

	return StructClass->GetName().GetCharArray().GetData();
}
