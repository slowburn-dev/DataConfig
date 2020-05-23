#pragma once

#include "CoreMinimal.h"
#include "DataConfigTypes.h"

namespace DataConfig
{

struct FWriterStorage;

struct DATACONFIGCORE_API FWriter
{
	FWriter();
	virtual ~FWriter();

	virtual FResult WriteBool(bool Value);
	virtual FResult WriteName(const FName& Value);
	virtual FResult WriteString(const FString& Value);

	virtual FResult WriteFloat(float Value);
	virtual FResult WriteDouble(double Value);

	virtual FResult WriteInt8(int8 Value);
	virtual FResult WriteInt16(int16 Value);
	virtual FResult WriteInt(int Value);
	virtual FResult WriteInt64(int64 Value);

	virtual FResult WriteByte(uint8 Value);
	virtual FResult WriteUInt16(uint16 Value);
	virtual FResult WriteUInt32(uint32 Value);
	virtual FResult WriteUInt64(uint64 Value);

	virtual FResult WriteStruct(const FName& StructName, FWriterStorage& OutWriter);
	virtual FResult WriteClass(const FName& ClassName, FWriterStorage& OutWriter);
	virtual FResult WriteMap(FWriterStorage& OutWriter);

	virtual FResult End();
};

struct DATACONFIGCORE_API FWriterStorage : private FNoncopyable
{
	using ImplStorageType = TAlignedStorage<64>::Type;
	ImplStorageType ImplStorage;

	FORCEINLINE FWriter* Get()
	{
		return reinterpret_cast<FWriter*>(this);
	}

	template<typename TWriter, typename... TArgs>
	TWriter* Emplace(TArgs&&... Args)
	{
		check(IsEmpty());
		return new (this)TWriter(Forward<TArgs>(Args)...);
	}

	FWriterStorage();
	~FWriterStorage();

private:
	bool IsEmpty();
};

}	// namespace DataConfig


