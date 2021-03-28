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
		::FDcResult Ret = (expr);			\
		if (!Ret.Ok()) {					\
			return Ret;						\
		}									\
	} while (0)

UENUM()
enum class EDcDataEntry : uint16
{
	Nil,

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

	//	Extension
	Blob,

	//	End
	Ended,
};

struct DATACONFIGCORE_API FDcStructStat
{
	FName Name;

	enum EFlag
	{
		None					= 0,
		WriteCheckName			= 0x1,
	};

	EFlag Flag = None;
};


struct DATACONFIGCORE_API FDcClassStat
{
	FName Name;

	enum class EControl
	{
		ReferenceOrNil,
		ExpandObject,
	};

	EControl Control;

	enum EFlag
	{
		None					= 0,
		WriteCheckName			= 0x1,
	};

	EFlag Flag = None;
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
		WriteCheckNameAndValue			= 1 << 1,
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
};


