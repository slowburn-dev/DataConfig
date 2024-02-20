#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcMacros.h"
#include "DcTypes.generated.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All);

struct DATACONFIGCORE_API FDcErrorCode
{
	uint16 CategoryID;
	uint16 ErrorID;
};

struct DATACONFIGCORE_API DC_NODISCARD FDcResult
{
	enum class EStatus : uint8
	{
		Ok,
		Error
	};

	EStatus Status;

	FORCEINLINE bool Ok() const
	{
		return Status == EStatus::Ok;
	}
};

FORCEINLINE FDcResult DcOk() {
	return FDcResult{ FDcResult::EStatus::Ok };
}

#define DC_TRY(expr)						\
	do {									\
		::FDcResult _DC_TRY_RET = (expr);		\
		if (!_DC_TRY_RET.Ok()) {				\
			return _DC_TRY_RET;				\
		}									\
	} while (0)

UENUM()
enum class EDcDataEntry : uint16
{
	None,

	Bool,
	Name,
	String,
	Text,
	Enum,

	Float,
	Double,

	Int8,
	Int16,
	Int32,
	Int64,

	UInt8,
	UInt16,
	UInt32,
	UInt64,

	//	Struct
	StructRoot,
	StructEnd,

	//	Class
	ClassRoot,
	ClassEnd,

	//	Map
	MapRoot,
	MapEnd,

	//	Array
	ArrayRoot,
	ArrayEnd,

	//	Set,
	SetRoot,
	SetEnd,

	//	Optional
	OptionalRoot,
	OptionalEnd,

	//	Reference
	ObjectReference,
	ClassReference,

	WeakObjectReference,
	LazyObjectReference,
	SoftObjectReference,
	SoftClassReference,
	InterfaceReference,

	//	Delegates
	Delegate,
	MulticastInlineDelegate,
	MulticastSparseDelegate,

	//	Field
	FieldPath,

	//	Extra
	Blob,

	//	Extension
	Extension,

	//	End
	Ended,
};

struct DATACONFIGCORE_API FDcStructAccess
{
	enum EFlag : uint8
	{
		None					= 0,
		WriteCheckName			= 0x1,
	};

	EFlag Flag = None;

	FName Name;

};

struct DATACONFIGCORE_API FDcClassAccess
{
	enum class EControl : uint8
	{
		Default,			//	default, determined by `Config.ShouldExpandObject()`
		ReferenceOrNone,	//	reference or none
		ExpandObject,		//	expand as sub object
	};

	EControl Control = EControl::Default;

	enum EFlag : uint8
	{
		None					= 0,
		WriteCheckName			= 0x1,
	};

	EFlag Flag = None;

	FName Name;
};

struct DATACONFIGCORE_API FDcEnumData
{
	FName Type;
	FName Name;

	union {
		int64 Signed64;
		uint64 Unsigned64;
	};

	bool bIsUnsigned;

	enum EFlag : uint16
	{
		None					= 0,
		WriteCheckType			= 1 << 0,
		WriteCheckNameAndValue	= 1 << 1,
		WriteCheckSign			= 1 << 2,
	};

	EFlag Flag = None;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FDcEnumData& Target)
	{
		Ar << Target.Type
			<< Target.Name
			<< Target.bIsUnsigned;

		if (Target.bIsUnsigned)
			Ar << Target.Signed64;
		else
			Ar << Target.Unsigned64;

		Ar << (uint16&)(Target.Flag);
		return Ar;
	}
};

struct DATACONFIGCORE_API FDcBlobViewData
{
	uint8* DataPtr;
	int32 Num;

	FORCEINLINE uint8 Get(int32 Ix) const
	{
		check(Ix >= 0 && Ix < Num);
		return DataPtr[Ix];
	}

	FORCEINLINE uint8* GetPtr(int32 Ix) const
	{
		check(Ix >= 0 && Ix < Num);
		return &DataPtr[Ix];
	}

	template<
		template<typename TElement, typename TAllocator> class TArr,
		typename TAlloc>
	static FORCEINLINE FDcBlobViewData From(const TArr<uint8, TAlloc>& Arr)
	{
		FDcBlobViewData Ret;
		Ret.DataPtr = (uint8*)Arr.GetData();
		Ret.Num = Arr.Num();
		return Ret;
	}
};

template<int32 N>
struct FDcFixedBytes
{
	uint8 Data[N];

	template<typename T> FORCEINLINE T& As() { return reinterpret_cast<T&>(Data); }

	template<
		template<typename TElement, typename TAllocator> class TArr,
		typename TAlloc>
	static FORCEINLINE FDcFixedBytes<N> From(const TArr<uint8, TAlloc>& Arr)
	{
		FDcFixedBytes<N> Ret;
		FPlatformMemory::Memcpy(Ret.Data, (uint8*)Arr.GetData(), N);
		return Ret;
	}
};

using FDcBytes2 = FDcFixedBytes<2>;
using FDcBytes4 = FDcFixedBytes<4>;
using FDcBytes8 = FDcFixedBytes<8>;
using FDcBytes16 = FDcFixedBytes<16>;
