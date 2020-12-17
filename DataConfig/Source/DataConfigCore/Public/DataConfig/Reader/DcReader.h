#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/DcTypes.h"

struct FDcDiagnostic;

struct DATACONFIGCORE_API FDcReader
{
	virtual ~FDcReader();

	virtual bool Coercion(EDcDataEntry ToEntry);

	virtual FDcResult PeekRead(EDcDataEntry* OutPtr);

	virtual FDcResult ReadNil();

	virtual FDcResult ReadBool(bool* OutPtr);
	virtual FDcResult ReadName(FName* OutPtr);
	virtual FDcResult ReadString(FString* OutPtr);
	virtual FDcResult ReadText(FText* OutPtr);
	virtual FDcResult ReadEnum(FDcEnumData* OutPtr);

	virtual FDcResult ReadStructRoot(FName* OutNamePtr);
	virtual FDcResult ReadStructEnd(FName* OutNamePtr);

	virtual FDcResult ReadClassRoot(FDcClassStat* OutClassPtr);
	virtual FDcResult ReadClassEnd(FDcClassStat* OutClassPtr);

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
	FDcResult ReadWeakObjectField(TWeakObjectPtr<TObject>* OutPtr);
	template<typename TObject>
	FDcResult ReadLazyObjectField(TLazyObjectPtr<TObject>* OutPtr);
	template<typename TObject>
	FDcResult ReadSoftObjectField(TSoftObjectPtr<TObject>* OutPtr);
	template<typename TClass>
	FDcResult ReadSoftClassField(TSoftClassPtr<TClass>* OutPtr);
	template<typename TInterface>
	FDcResult ReadInterfaceField(TScriptInterface<TInterface>* OutPtr);

	virtual FDcResult ReadFieldPath(FFieldPath* OutPtr);

	template<typename TProperty>
	FDcResult ReadPropertyField(TFieldPath<TProperty>* OutPtr);

	virtual FDcResult ReadDelegate(FScriptDelegate* OutPtr);
	virtual FDcResult ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr);
	virtual FDcResult ReadMulticastSparseDelegate(FSparseDelegate* OutPtr);

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

	virtual FDcResult ReadBlob(FDcBlobViewData* OutPtr);

	virtual void FormatDiagnostic(FDcDiagnostic& Diag);

	FORCEINLINE friend FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FDcReader& Self)
	{
		Self.FormatDiagnostic(Diag);
		return Diag;
	}

};

template<typename TObject>
FDcResult FDcReader::ReadWeakObjectField(TWeakObjectPtr<TObject>* OutPtr)
{
	//	TODO c++20 `is_layout_compatible`
	static_assert(sizeof(FWeakObjectPtr) == sizeof(TWeakObjectPtr<TObject>), "TWeakkObjectPtr should have same memory layout as FWeakObjectPtr");

	FWeakObjectPtr* WeakOutPtr = (FWeakObjectPtr*)OutPtr;
	return ReadWeakObjectReference(WeakOutPtr);
}

template<typename TObject>
FDcResult FDcReader::ReadLazyObjectField(TLazyObjectPtr<TObject>* OutPtr)
{
	FLazyObjectPtr LazyPtr;
	DC_TRY(ReadLazyObjectReference(&LazyPtr));

	ReadOut(OutPtr, LazyPtr.GetUniqueID());
	return DcOk();
}

template<typename TObject>
FDcResult FDcReader::ReadSoftObjectField(TSoftObjectPtr<TObject>* OutPtr)
{
	FSoftObjectPath SoftPath;
	DC_TRY(ReadSoftObjectReference(&SoftPath));

	ReadOut(OutPtr, SoftPath);
	return DcOk();
}

template<typename TClass>
FDcResult FDcReader::ReadSoftClassField(TSoftClassPtr<TClass>* OutPtr)
{
	FSoftClassPath SoftPath;
	DC_TRY(ReadSoftClassReference(&SoftPath));

	ReadOut(OutPtr, SoftPath);
	return DcOk();
}

template<typename TInterface>
FDcResult FDcReader::ReadInterfaceField(TScriptInterface<TInterface>* OutPtr)
{
	FScriptInterface ScriptInterface;
	DC_TRY(ReadInterfaceReference(OutPtr));

	ReadOut(OutPtr, (const TScriptInterface<TInterface>&)ScriptInterface);
	return DcOk();
}

template<typename TProperty>
FDcResult FDcReader::ReadPropertyField(TFieldPath<TProperty>* OutPtr)
{
	static_assert(sizeof(FFieldPath) == sizeof(TFieldPath<TProperty>), "FFieldPath and TFieldPath should have identical layout");
	FFieldPath* FieldPathPtr = (FFieldPath*)OutPtr;
	return ReadFieldPath(FieldPathPtr);
}

