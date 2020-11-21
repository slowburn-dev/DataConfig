#pragma once

#include "CoreMinimal.h"
#include "DcTypes.generated.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All);

//	TODO move these into a seperate header
//	global initializer and shutdown
enum class EDcInitializeAction
{
	Minimal,
	SetAsConsole,
};

DATACONFIGCORE_API void DcStartUp(EDcInitializeAction InAction = EDcInitializeAction::Minimal);
DATACONFIGCORE_API void DcShutDown();
DATACONFIGCORE_API bool DcIsInitialized();

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


//	std::aligned_storage stub
//	https://devdocs.io/cpp/types/aligned_storage
template <size_t _Len, size_t _Align = MIN_ALIGNMENT>
struct TDcAlignedStorage
{
	struct Type
	{
		unsigned char Data[Align(_Len, _Align)];
	};
};

UENUM()
enum class EDcDataEntry
{
	//	Data Type
	Nil,	 //	!! note that this actually doesn't exist in UE4 property data model

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

	//	Meta
	Ended, // or error or invalid state, 
};

enum class EDcDataReference
{
	NullReference,
	ExternalReference,
	ExpandObject,
};

struct FDcClassPropertyStat
{
	FName Name;
	EDcDataReference Reference;
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

