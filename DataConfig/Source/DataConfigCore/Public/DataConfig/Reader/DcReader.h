#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct FDcDiagnostic;

struct DATACONFIGCORE_API FDcReader
{
	virtual ~FDcReader();

	virtual bool Coercion(EDcDataEntry ToEntry);

	virtual FDcResult ReadNext(EDcDataEntry* OutPtr);

	virtual FDcResult ReadNil();

	virtual FDcResult ReadBool(bool* OutPtr);
	virtual FDcResult ReadName(FName* OutPtr);
	virtual FDcResult ReadString(FString* OutPtr);
	virtual FDcResult ReadText(FText* OutPtr);
	virtual FDcResult ReadEnum(FDcEnumData* OutPtr);

	virtual FDcResult ReadStructRoot(FName* OutNamePtr);
	virtual FDcResult ReadStructEnd(FName* OutNamePtr);

	virtual FDcResult ReadClassRoot(FDcObjectPropertyStat* OutClassPtr);
	virtual FDcResult ReadClassEnd(FDcObjectPropertyStat* OutClassPtr);

	virtual FDcResult ReadMapRoot();
	virtual FDcResult ReadMapEnd();

	virtual FDcResult ReadArrayRoot();
	virtual FDcResult ReadArrayEnd();

	virtual FDcResult ReadSetRoot();
	virtual FDcResult ReadSetEnd();

	virtual FDcResult ReadObjectReference(UObject** OutPtr);
	virtual FDcResult ReadClassReference(UClass** OutPtr);

	virtual FDcResult ReadWeakObjectReference(FWeakObjectPtr* OutPtr);
	virtual FDcResult ReadLazyObjectReference(FLazyObjectPtr* OutPtr);
	virtual FDcResult ReadSoftObjectReference(FSoftObjectPath* OutPtr);
	virtual FDcResult ReadSoftClassReference(FSoftClassPath* OutPtr);
	virtual FDcResult ReadInterfaceReference(FScriptInterface* OutPtr);

	template<typename TObject>
	FDcResult ReadWeakObjectPtr(TWeakObjectPtr<TObject>* OutPtr);
	template<typename TObject>
	FDcResult ReadLazyObjectPtr(TLazyObjectPtr<TObject>* OutPtr);
	template<typename TObject>
	FDcResult ReadSoftObjectPtr(TSoftObjectPtr<TObject>* OutPtr);
	template<typename TClass>
	FDcResult ReadSoftClassPtr(TSoftClassPtr<TClass>* OutPtr);
	template<typename TInterface>
	FDcResult ReadInterfacePtr(TScriptInterface<TInterface>* OutPtr);

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

	virtual void FormatDiagnostic(FDcDiagnostic& Diag);

	FORCEINLINE friend FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FDcReader& Self)
	{
		Self.FormatDiagnostic(Diag);
		return Diag;
	}

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

template<typename TObject>
FDcResult FDcReader::ReadWeakObjectPtr(TWeakObjectPtr<TObject>* OutPtr)
{
	//	TODO c++20 `is_layout_compatible`
	static_assert(sizeof(FWeakObjectPtr) == sizeof(TWeakObjectPtr<TObject>), "TWeakkObjectPtr should have same memory layout as FWeakObjectPtr");

	FWeakObjectPtr* WeakOutPtr = (FWeakObjectPtr*)OutPtr;
	return ReadWeakObjectReference(WeakOutPtr);
}

template<typename TObject>
FDcResult FDcReader::ReadLazyObjectPtr(TLazyObjectPtr<TObject>* OutPtr)
{
	FLazyObjectPtr LazyPtr;
	DC_TRY(ReadLazyObjectReference(&LazyPtr));

	if (OutPtr)
	{
		*OutPtr = LazyPtr.GetUniqueID();
	}
	return DcOk();
}

template<typename TObject>
FDcResult FDcReader::ReadSoftObjectPtr(TSoftObjectPtr<TObject>* OutPtr)
{
	FSoftObjectPath SoftPath;
	DC_TRY(ReadSoftObjectReference(&SoftPath));

	if (OutPtr)
	{
		//*OutPtr = MoveTemp(SoftPath);
		*OutPtr = SoftPath;
	}
	return DcOk();
}

template<typename TClass>
FDcResult FDcReader::ReadSoftClassPtr(TSoftClassPtr<TClass>* OutPtr)
{
	FSoftClassPath SoftPath;
	DC_TRY(ReadSoftClassReference(&SoftPath));

	if (OutPtr)
	{
		*OutPtr = SoftPath;
	}
	return DcOk();
}

template<typename TInterface>
FDcResult FDcReader::ReadInterfacePtr(TScriptInterface<TInterface>* OutPtr)
{
	FScriptInterface ScriptInterface;
	DC_TRY(ReadInterfaceReference(OutPtr));

	if (OutPtr)
	{
		*OutPtr = (const TScriptInterface<TInterface>&)ScriptInterface;
	}
	return DcOk();
}

