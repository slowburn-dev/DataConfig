#pragma once

#include "CoreMinimal.h"
#include "PropertyCommon/PropertyDatum.h"
#include "Writer/Writer.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FBasePropertyWriter : public FWriter, private FNoncopyable
{
	FPropertyDatum ActiveDatum;

	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;
	FResult WriteStruct(const FName& StructName, FWriterStorage& OutWriter);

	virtual FResult PreWriteCheck();
};

struct DATACONFIGCORE_API FPrimitivePropertyWriter : public FBasePropertyWriter
{
	using Super = FBasePropertyWriter;

	FPrimitivePropertyWriter(void* PrimitivePtr, UProperty* Property);
};

struct DATACONFIGCORE_API FStructMapWriter : public FBasePropertyWriter
{
	using Super = FBasePropertyWriter;

	FStructMapWriter(void* InStructPtr, UScriptStruct* InStructClass);

	void* StructPtr;
	UScriptStruct* StructClass;

	enum class EState : uint8
	{
		WaitWriteKey, 
		WaitWriteValue,
		End,
	};
	EState State;

	FResult WriteName(const FName& Value) override;
	FResult PreWriteCheck() override;

	FResult End() override;

	FPropertyDatum FindChildDatumByName(const FName& ChildName);
};

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




