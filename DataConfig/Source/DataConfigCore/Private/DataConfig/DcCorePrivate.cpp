#include "CoreMinimal.h"
#include "DcCorePrivate.h"


#if !IS_MONOLITHIC

//	amend module boilerplate on modular builds
PER_MODULE_BOILERPLATE

#endif


namespace DcCorePrivate {

void FStringNewlineDevice::Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const class FName& Category) 
{
	FString::operator+=((TCHAR*)InData);
	*this += LINE_TERMINATOR;
}

FString& FStringNewlineDevice::operator+=(const FString& Other)
{
	return FString::operator+=(Other);
}


} // namespace DcCorePrivate


