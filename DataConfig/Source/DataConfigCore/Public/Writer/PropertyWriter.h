#pragma once

#include "CoreMinimal.h"
#include "Writer/Writer.h"

namespace DataConfig
{

/*
struct DATACONFIGCORE_API FPropertyDatum
{
	UField* Property;
	void* DataPtr;
};

struct DATACONFIGCORE_API FPropertyWriter : public FWriter, private FNoncopyable
{
	FPropertyWriter(UField* Property);

	FPropertyDatum ActiveDatum;
};
*/

struct DATACONFIGCORE_API FPropertyWriter : public FWriter, private FNoncopyable
{
	FPropertyWriter();
	virtual ~FPropertyWriter();

	FPropertyWriter(UObject* ClassObject);
	FPropertyWriter(UObject* ClassObject, UProperty* Property);
	FPropertyWriter(void* StructPtr, UScriptStruct* StructClass);
	FPropertyWriter(void* StructPtr, UScriptStruct* StructClass, UProperty* Property);
	FPropertyWriter(void* PrimitivePtr, UProperty* Property);

	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;

	struct FStructMapWriter;
	FResult WriteStruct(const FName& StructName, FWriterStorage& OutWriter) override;

	using ImplStorageType = TAlignedStorage<56>::Type;
	ImplStorageType ImplStorage;
};

struct FPropertyWriter::FStructMapWriter : public FPropertyWriter 
{
	FStructMapWriter(void* StructPtr, UScriptStruct* StructClass);

	FResult End() override;
};


}	// namespace DataConfig




