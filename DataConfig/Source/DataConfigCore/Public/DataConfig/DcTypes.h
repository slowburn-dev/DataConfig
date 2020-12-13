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

struct DATACONFIGCORE_API FDcResult
{
	//	TODO when benchmark is setup, try figure out whether
	//		 changing this to uint32 affects anything
	enum class EStatus : uint8
	{
		Ok,
		Error
	};

	EStatus Status;

	FORCEINLINE bool Ok()
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
	//	Data Type
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
	ObjectReference,	// `ObjectReference` always happen between `ClassRoot/ClassEnd`
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

	//	Meta
	Ended, // or error or invalid state, 
};

enum class EDcObjectPropertyControl
{
	ExternalReference,
	ExpandObject,
};

struct FDcObjectPropertyStat
{
	FName Name;
	EDcObjectPropertyControl Reference;
};

struct FDcEnumData
{
	FName Type;
	FName Name;
	int64 Value;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FDcEnumData& Target)
	{
		Ar << Target.Type
			<< Target.Name
			<< Target.Value;

		return Ar;
	}
};

struct FDcBlobViewData
{
	uint8* DataPtr;
	int32 Num;
};


