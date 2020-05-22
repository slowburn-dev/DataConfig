#pragma once

#include "CoreMinimal.h"

namespace DataConfig
{


struct DATACONFIGCORE_API FPropertyWriter : private FNoncopyable
{
	FPropertyWriter();
	~FPropertyWriter();

	FPropertyWriter(UObject* ClassObject);
	FPropertyWriter(UObject* ClassObject, UProperty* Property);
	FPropertyWriter(void* StructPtr, UScriptStruct* StructClass);
	FPropertyWriter(void* StructPtr, UScriptStruct* StructClass, UProperty* Property);
	FPropertyWriter(void* PrimitivePtr, UProperty* Property);


};










}	// namespace DataConfig




