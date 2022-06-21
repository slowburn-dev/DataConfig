#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/DcTypes.h"

struct FDcDiagnostic;

struct DATACONFIGCORE_API FDcReader
{
	virtual ~FDcReader();

	virtual FDcResult Coercion(EDcDataEntry ToEntry, bool* OutPtr);

	virtual FDcResult PeekRead(EDcDataEntry* OutPtr);

	virtual FDcResult ReadNil();

	virtual FDcResult ReadBool(bool* OutPtr);
	virtual FDcResult ReadName(FName* OutPtr);
	virtual FDcResult ReadString(FString* OutPtr);
	virtual FDcResult ReadText(FText* OutPtr);
	virtual FDcResult ReadEnum(FDcEnumData* OutPtr);

	template<typename TEnum>
	FDcResult ReadEnumField(TEnum* OutPtr);

	virtual FDcResult ReadStructRootAccess(FDcStructAccess& Access);
	virtual FDcResult ReadStructEndAccess(FDcStructAccess& Access);

	virtual FDcResult ReadClassRootAccess(FDcClassAccess& Access);
	virtual FDcResult ReadClassEndAccess(FDcClassAccess& Access);

	FDcResult ReadStructRoot();
	FDcResult ReadStructEnd();

	FDcResult ReadClassRoot();
	FDcResult ReadClassEnd();

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
	virtual FDcResult ReadSoftObjectReference(FSoftObjectPtr* OutPtr);
	virtual FDcResult ReadSoftClassReference(FSoftObjectPtr* OutPtr);
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
	virtual FDcResult ReadMulticastSparseDelegate(FMulticastScriptDelegate* OutPtr);

	template<typename MulticastDelegate, typename OwningClass, typename DelegateInfoClass>
	FDcResult ReadSparseDelegateField(TSparseDynamicDelegate<MulticastDelegate, OwningClass, DelegateInfoClass>* OutPtr);

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

#if ENGINE_MAJOR_VERSION == 5
	template<typename TObject>
	FDcResult ReadTObjectPtr(TObjectPtr<TObject>* OutPtr);
#endif

	static FName ClassId(); 
	virtual FName GetId();

	template<typename T>
	T* CastById();

	template<typename T>
	T* CastByIdChecked();
};


#include "DataConfig/Reader/DcReader.inl"

