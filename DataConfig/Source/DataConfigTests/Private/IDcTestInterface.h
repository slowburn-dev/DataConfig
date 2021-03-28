#pragma once

#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "IDcTestInterface.generated.h"

//	these 2 classes must be in one standalone header or it's a guarenteed UHT cyclic header reference

UINTERFACE()
class UDcTestInterface1 : public UInterface
{
	GENERATED_BODY()
};

class IDcTestInterface1
{
	GENERATED_BODY()
};

