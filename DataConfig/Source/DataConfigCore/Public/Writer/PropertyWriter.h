#pragma once

#include "CoreMinimal.h"

namespace DataConfig
{


struct DATACONFIGCORE_API FPropertyWriter : private FNoncopyable
{
	FPropertyWriter();
	virtual ~FPropertyWriter();

	FPropertyWriter(UObject* ClassObject);
	FPropertyWriter(UObject* ClassObject, UProperty* Property);
	FPropertyWriter(void* StructPtr, UScriptStruct* StructClass);
	FPropertyWriter(void* StructPtr, UScriptStruct* StructClass, UProperty* Property);
	FPropertyWriter(void* PrimitivePtr, UProperty* Property);

	FResult WriteBool(bool Value);
	FResult WriteName(const FName& Value);
	FResult WriteString(const FString& Value);

	struct FStructMapWriter;

	FResult WriteStruct(const FName& StructName, FStructMapWriter& OutWriter);


	using ImplStorageType = TAlignedStorage<56>::Type;
	ImplStorageType ImplStorage;
};

struct FPropertyWriter::FStructMapWriter : public FPropertyWriter 
{
	void Emplace(void* StructPtr, UScriptStruct* StructClass);

	FResult End();
};




}	// namespace DataConfig




