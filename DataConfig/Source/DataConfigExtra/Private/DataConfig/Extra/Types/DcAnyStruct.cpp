#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

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

void FDcAnyStruct::DebugDump()
{
	FString Dumped = DcAutomationUtils::DumpFormat(FDcPropertyDatum(StructClass, DataPtr));
	FPlatformMisc::LowLevelOutputDebugString(*Dumped);
}

