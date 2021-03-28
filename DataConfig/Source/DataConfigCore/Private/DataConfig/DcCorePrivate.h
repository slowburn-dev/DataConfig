#pragma once

#include "CoreMinimal.h"

///	sharing private code

namespace DcCorePrivate {

struct FStringNewlineDevice : public FString, public FOutputDevice
{
	FStringNewlineDevice()
		: FString()
	{}

	void Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const class FName& Category) override;
	virtual FString& operator+=(const FString& Other);
};


} // namespace DcCorePrivate

