#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DcInlineStruct.generated.h"

/// Inline struct
///	- Stores arbitrary struct with inline buffer
///	- Can be only assigned once, can be copied but not moved.

template<int32 BUFLEN>
struct TDcInlineStructBase
{
	constexpr static int32 _BUFLEN = BUFLEN; 
	constexpr static int32 _CLASS_OFFSET = _BUFLEN - sizeof(void*);

	//	[<struct body>, UScriptStruct*]
	TAlignedBytes<_BUFLEN, 16u> Buffer;

	TDcInlineStructBase() { FPlatformMemory::Memzero(&Buffer, _BUFLEN); }

	template<typename TStruct, typename... TArgs>
	TStruct* Emplace(TArgs&&... Args)
	{
		UScriptStruct** StructPtr = GetStructPtr();  
		//	can only assign once
		check(!IsValid());
		UScriptStruct* Struct = TBaseStructure<TStruct>::Get();
		check(Struct->GetStructureSize() <= _CLASS_OFFSET);
		*StructPtr = Struct;
		new (&Buffer) TStruct(Forward<TArgs>(Args)...);
		return (TStruct*)(&Buffer);
	}

	FORCEINLINE void CopyFromOther(const TDcInlineStructBase& InOther)
	{
		if (UScriptStruct* Struct = *InOther.GetStructPtr())
		{
			Struct->InitializeStruct(&Buffer);
			Struct->CopyScriptStruct(&Buffer, &InOther.Buffer);
			*GetStructPtr() = Struct;
		}
		else
		{
			// copy non initialized is fine
			FPlatformMemory::Memzero(&Buffer, _BUFLEN);
		}
	}

	TDcInlineStructBase(const TDcInlineStructBase& InOther)
	{
		CopyFromOther(InOther);
	}

	TDcInlineStructBase& operator=(const TDcInlineStructBase& InOther)
	{
		if (this != &InOther)
		{
			//	can be only set once
			check(!IsValid());
			CopyFromOther(InOther);
		}
	
		return *this;
	}


	template<class TStruct>
	TStruct* GetChecked() const
	{
		check(TBaseStructure<TStruct>::Get() == *GetStructPtr());
		return (TStruct*)(&Buffer);
	}

	UScriptStruct** GetStructPtr() const
	{
		return (UScriptStruct**)((uint8*)&Buffer + _CLASS_OFFSET);
	}

	bool IsValid() const
	{
		return *GetStructPtr() != nullptr;
	}

	~TDcInlineStructBase()
	{
		if (IsValid())
			(*GetStructPtr())->DestroyStruct(&Buffer);
	}

	///	Dump data to output. Intended to be called in debugger immediate.
	void DebugDump()
	{
		DcAutomationUtils::DumpToLowLevelDebugOutput(FDcPropertyDatum(*GetStructPtr(), &Buffer));
	}
};

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcInlineStruct64
#if CPP
	: public TDcInlineStructBase<64>
#endif
{
	GENERATED_BODY()

	using TDcInlineStructBase::TDcInlineStructBase;
};


USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcInlineStruct128
#if CPP
	: public TDcInlineStructBase<128>
#endif
{
	GENERATED_BODY()

	using TDcInlineStructBase::TDcInlineStructBase;
};

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcInlineStruct256
#if CPP
	: public TDcInlineStructBase<256>
#endif
{
	GENERATED_BODY()

	using TDcInlineStructBase::TDcInlineStructBase;
};

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcInlineStruct512
#if CPP
	: public TDcInlineStructBase<512>
#endif
{
	GENERATED_BODY()

	using TDcInlineStructBase::TDcInlineStructBase;
};


