#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointerInternals.h"
#include "DcAnyStruct.generated.h"

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcAnyStruct
{
	GENERATED_BODY()

	struct DATACONFIGEXTRA_API AnyStructReferenceController : public SharedPointerInternals::FReferenceControllerBase
	{
		AnyStructReferenceController(FDcAnyStruct* InAnyStruct)
			: AnyStruct(InAnyStruct)
		{}

		void DestroyObject() override;
		FDcAnyStruct* AnyStruct;
	};

	FDcAnyStruct(SharedPointerInternals::FNullTag* = nullptr)
		: DataPtr(nullptr)
		, StructClass(nullptr)
		, SharedReferenceCount(nullptr)
	{}

	FDcAnyStruct(void* InDataPtr, UScriptStruct* InStructClass)
		: DataPtr(InDataPtr)
		, StructClass(InStructClass)
		, SharedReferenceCount(new AnyStructReferenceController(this))
	{}

	FDcAnyStruct(FDcAnyStruct const& InOther)
		: DataPtr(InOther.DataPtr)
		, StructClass(InOther.StructClass)
		, SharedReferenceCount(InOther.SharedReferenceCount)
	{}

	FDcAnyStruct(FDcAnyStruct&& InOther)
		: DataPtr(InOther.DataPtr)
		, StructClass(InOther.StructClass)
		, SharedReferenceCount(MoveTemp(InOther.SharedReferenceCount))
	{
		InOther.DataPtr = nullptr;
		InOther.StructClass = nullptr;
	}

	void Reset()
	{
		*this = FDcAnyStruct();
		check(DataPtr == nullptr && StructClass == nullptr);
	}

	FDcAnyStruct& operator=(SharedPointerInternals::FNullTag*)
	{
		Reset();
		return *this;
	}

	FDcAnyStruct& operator=(FDcAnyStruct const& InOther)
	{
		FDcAnyStruct Temp = InOther;
		Swap(Temp, *this);
		return *this;
	}

	FDcAnyStruct& operator=(FDcAnyStruct&& InOther)
	{
		if (this != &InOther)
		{
			DataPtr = InOther.DataPtr;
			InOther.DataPtr = nullptr;
			InOther.StructClass = nullptr;
			SharedReferenceCount = MoveTemp(InOther.SharedReferenceCount);
		}

		return *this;
	}

	const int32 GetSharedReferenceCount() const
	{
		return SharedReferenceCount.GetSharedReferenceCount();
	}

	const bool IsValid() const
	{
		return SharedReferenceCount.IsValid();
	}

	template<class T>
	FDcAnyStruct(T* StructPtr)
		: DataPtr((void*)StructPtr)
		, StructClass(T::StaticStruct())
		, SharedReferenceCount(new AnyStructReferenceController(this))
	{}

	template<class T>
	T* GetChecked() const
	{
		check(T::StaticStruct() == StructClass);
		return (T*)(DataPtr);
	}

	void* DataPtr;
	UScriptStruct* StructClass;
	SharedPointerInternals::FSharedReferencer<ESPMode::Fast> SharedReferenceCount;

	TCHAR* GetTypeNameLiteral();
};








