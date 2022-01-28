#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "DataConfig/DcTypes.h"

struct FDcDiagnostic;

struct DATACONFIGCORE_API FDcWriter
{
	virtual ~FDcWriter();

	virtual FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk);

	virtual FDcResult WriteNil();
	virtual FDcResult WriteBool(bool Value);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteString(const FString& Value);
	virtual FDcResult WriteText(const FText& Value);
	virtual FDcResult WriteEnum(const FDcEnumData& Value);

	template<typename TEnum>
	FDcResult WriteEnumField(const TEnum& Value);

	virtual FDcResult WriteStructRootAccess(FDcStructAccess& Access);
	virtual FDcResult WriteStructEndAccess(FDcStructAccess& Access);

	virtual FDcResult WriteClassRootAccess(FDcClassAccess& Access);
	virtual FDcResult WriteClassEndAccess(FDcClassAccess& Access);

	FDcResult WriteStructRoot();
	FDcResult WriteStructEnd();

	FDcResult WriteClassRoot();
	FDcResult WriteClassEnd();

	virtual FDcResult WriteMapRoot();
	virtual FDcResult WriteMapEnd();

	virtual FDcResult WriteArrayRoot();
	virtual FDcResult WriteArrayEnd();

	virtual FDcResult WriteSetRoot();
	virtual FDcResult WriteSetEnd();

	virtual FDcResult WriteObjectReference(const UObject* Value);
	virtual FDcResult WriteClassReference(const UClass* Value);

	virtual FDcResult WriteWeakObjectReference(const FWeakObjectPtr& Value);
	virtual FDcResult WriteLazyObjectReference(const FLazyObjectPtr& Value);
	virtual FDcResult WriteSoftObjectReference(const FSoftObjectPtr& Value);
	virtual FDcResult WriteSoftClassReference(const FSoftObjectPtr& Value);
	virtual FDcResult WriteInterfaceReference(const FScriptInterface& Value);

	template<typename TObject>
	FDcResult WriteWeakObjectField(const TWeakObjectPtr<TObject>& Value);
	template<typename TObject>
	FDcResult WriteLazyObjectField(const TLazyObjectPtr<TObject>& Value);
	template<typename TObject>
	FDcResult WriteSoftObjectField(const TSoftObjectPtr<TObject>& Value);
	template<typename TClass>
	FDcResult WriteSoftClassField(const TSoftClassPtr<TClass>& Value);
	template<typename TInterface>
	FDcResult WriteInterfaceField(const TScriptInterface<TInterface>& Value);

	virtual FDcResult WriteFieldPath(const FFieldPath& Value);
	template<typename TProperty>
	FDcResult WritePropertyField(const TFieldPath<TProperty>& Value);

	virtual FDcResult WriteDelegate(const FScriptDelegate& Value);
	virtual FDcResult WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value);
	virtual FDcResult WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value);

	template<typename MulticastDelegate, typename OwningClass, typename DelegateInfoClass>
	FDcResult WriteSparseDelegateField(const TSparseDynamicDelegate<MulticastDelegate, OwningClass, DelegateInfoClass>& Value);

	virtual FDcResult WriteInt8(const int8& Value);
	virtual FDcResult WriteInt16(const int16& Value);
	virtual FDcResult WriteInt32(const int32& Value);
	virtual FDcResult WriteInt64(const int64& Value);

	virtual FDcResult WriteUInt8(const uint8& Value);
	virtual FDcResult WriteUInt16(const uint16& Value);
	virtual FDcResult WriteUInt32(const uint32& Value);
	virtual FDcResult WriteUInt64(const uint64& Value);

	virtual FDcResult WriteFloat(const float& Value);
	virtual FDcResult WriteDouble(const double& Value);

	virtual FDcResult WriteBlob(const FDcBlobViewData& Value);

	virtual void FormatDiagnostic(FDcDiagnostic& Diag);

	FORCEINLINE friend FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FDcWriter& Self)
	{
		Self.FormatDiagnostic(Diag);
		return Diag;
	}

#if ENGINE_MAJOR_VERSION == 5
	template<typename TObject>
	FDcResult WriteTObjectPtr(const TObjectPtr<TObject>& Value);
#endif

	static FName ClassId(); 
	virtual FName GetId();

	template<typename T>
	T* CastById();
};


#include "DataConfig/Writer/DcWriter.inl"

