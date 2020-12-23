#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct FDcDiagnostic;

struct DATACONFIGCORE_API FDcWriter
{
	FDcWriter();
	virtual ~FDcWriter();

	virtual FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk);

	virtual FDcResult WriteNil();
	virtual FDcResult WriteBool(bool Value);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteString(const FString& Value);
	virtual FDcResult WriteText(const FText& Value);
	virtual FDcResult WriteEnum(const FDcEnumData& Value);

	virtual FDcResult WriteStructRoot(const FDcStructStat& Struct);
	virtual FDcResult WriteStructEnd(const FDcStructStat& Struct);

	virtual FDcResult WriteClassRoot(const FDcClassStat& Class);
	virtual FDcResult WriteClassEnd(const FDcClassStat& Class);

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
	virtual FDcResult WriteSoftObjectReference(const FSoftObjectPath& Value);
	virtual FDcResult WriteSoftClassReference(const FSoftClassPath& Value);
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

};

template<typename TObject>
FDcResult FDcWriter::WriteWeakObjectField(const TWeakObjectPtr<TObject>& Value)
{
	//	TODO c++20 `is_layout_compatible`
	static_assert(sizeof(FWeakObjectPtr) == sizeof(TWeakObjectPtr<TObject>), "TWeakkObjectPtr should have same memory layout as FWeakObjectPtr");

	const FWeakObjectPtr& WeakPtr = (const FWeakObjectPtr&)Value;
	return WriteWeakObjectReference(WeakPtr);
}

template<typename TObject>
FDcResult FDcWriter::WriteLazyObjectField(const TLazyObjectPtr<TObject>& Value)
{
	//	LazyPtr is missing a copy constructor, can only do assign
	FLazyObjectPtr LazyPtr;
	LazyPtr = Value.GetUniqueID();
	return WriteLazyObjectReference(LazyPtr);
}

template<typename TObject>
FDcResult FDcWriter::WriteSoftObjectField(const TSoftObjectPtr<TObject>& Value)
{
	FSoftObjectPath SoftPath(Value.GetUniqueID());
	return WriteSoftObjectReference(SoftPath);
}

template<typename TClass>
FDcResult FDcWriter::WriteSoftClassField(const TSoftClassPtr<TClass>& Value)
{
	FSoftClassPath SoftPath((const FSoftClassPath&)Value.GetUniqueID());
	return WriteSoftClassReference(SoftPath);
}

template<typename TInterface>
FDcResult FDcWriter::WriteInterfaceField(const TScriptInterface<TInterface>& Value)
{
	FScriptInterface ScriptInterface(Value);
	return WriteInterfaceReference(ScriptInterface);
}

template<typename TProperty>
FDcResult FDcWriter::WritePropertyField(const TFieldPath<TProperty>& Value)
{
	static_assert(sizeof(FFieldPath) == sizeof(TFieldPath<TProperty>), "FFieldPath and TFieldPath should have identical layout");

	const FFieldPath& FieldPath = (const FFieldPath&)Value;
	return WriteFieldPath(FieldPath);
}

namespace DcPropertyUtils
{
	DATACONFIGCORE_API FDcResult FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset, FProperty*& OutValue);
}

template<typename MulticastDelegate, typename OwningClass, typename DelegateInfoClass>
FDcResult FDcWriter::WriteSparseDelegateField(const TSparseDynamicDelegate<MulticastDelegate, OwningClass, DelegateInfoClass>& Value)
{
	FProperty* Property;
	DC_TRY(DcPropertyUtils::FindEffectivePropertyByOffset(
		OwningClass::StaticClass(),
		DelegateInfoClass::template GetDelegateOffset<OwningClass>(),
		Property
	));

	FMulticastSparseDelegateProperty* SparseProperty = CastFieldChecked<FMulticastSparseDelegateProperty>(Property);
	const FMulticastScriptDelegate* MulticastDelegate = SparseProperty->GetMulticastDelegate(&Value);

	if (MulticastDelegate == nullptr)
	{
		FMulticastScriptDelegate Empty;
		return WriteMulticastSparseDelegate(Empty);
	}
	else
	{
		return WriteMulticastSparseDelegate(*MulticastDelegate);
	}
}

