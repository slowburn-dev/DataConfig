#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcReader
{
	virtual ~FDcReader();

	virtual bool Coercion(EDcDataEntry ToEntry);

	virtual FDcResult ReadNext(EDcDataEntry* OutPtr);

	virtual FDcResult ReadNil();

	virtual FDcResult ReadBool(bool* OutPtr);
	virtual FDcResult ReadName(FName* OutPtr);
	virtual FDcResult ReadString(FString* OutPtr);

	virtual FDcResult ReadStructRoot(FName* OutNamePtr);
	virtual FDcResult ReadStructEnd(FName* OutNamePtr);

	virtual FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr);
	virtual FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr);

	virtual FDcResult ReadMapRoot();
	virtual FDcResult ReadMapEnd();

	virtual FDcResult ReadArrayRoot();
	virtual FDcResult ReadArrayEnd();

	virtual FDcResult ReadSetRoot();
	virtual FDcResult ReadSetEnd();

	virtual FDcResult ReadReference(UObject** OutPtr);

	virtual FDcResult ReadInt8(int8* OutPtr);
	virtual FDcResult ReadInt16(int16* OutPtr);
	virtual FDcResult ReadInt32(int32* OutPtr);
	virtual FDcResult ReadInt64(int64* OutPtr);

	virtual FDcResult ReadUInt8(uint8* OutPtr);
	virtual FDcResult ReadUInt16(uint16* OutPtr);
	virtual FDcResult ReadUInt32(uint32* OutPtr);
	virtual FDcResult ReadUInt64(uint64* OutPtr);

	virtual FDcResult ReadFloat(float* OutPtr);
	virtual FDcResult ReadDouble(double* OutPtr);

	//	shorthand for optional reading values
	template<typename T1, typename T2>
	FORCEINLINE static void ReadOut(T1*& OutPtr, T2&& Value)
	{
		if (OutPtr)
		{
			*OutPtr = Forward<T2>(Value);
		}
	}
};


