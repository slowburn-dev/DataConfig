#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct FDcDiagnostic;

struct DATACONFIGCORE_API FDcWriter
{
	FDcWriter();
	virtual ~FDcWriter();

	virtual FDcResult WriteNext(EDcDataEntry Next);

	virtual FDcResult WriteNil();
	virtual FDcResult WriteBool(bool Value);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteString(const FString& Value);
	virtual FDcResult WriteText(const FText& Value);
	virtual FDcResult WriteEnum(const FDcEnumData& Value);

	virtual FDcResult WriteStructRoot(const FName& Name);
	virtual FDcResult WriteStructEnd(const FName& Name);

	virtual FDcResult WriteClassRoot(const FDcObjectPropertyStat& Class);
	virtual FDcResult WriteClassEnd(const FDcObjectPropertyStat& Class);

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

	template<typename TObject>
	FDcResult WriteWeakObjectPtr(const TWeakObjectPtr<TObject>& Value);
	template<typename TObject>
	FDcResult WriteLazyObjectPtr(const TLazyObjectPtr<TObject>& Value);
	template<typename TObject>
	FDcResult WriteSoftObjectPtr(const TSoftObjectPtr<TObject>& Value);

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

	virtual void FormatDiagnostic(FDcDiagnostic& Diag);

	FORCEINLINE friend FDcDiagnostic& operator<<(FDcDiagnostic& Diag, FDcWriter& Self)
	{
		Self.FormatDiagnostic(Diag);
		return Diag;
	}

};

template<typename TObject>
FDcResult FDcWriter::WriteWeakObjectPtr(const TWeakObjectPtr<TObject>& Value)
{
	//	TODO c++20 `is_layout_compatible`
	static_assert(sizeof(FWeakObjectPtr) == sizeof(TWeakObjectPtr<TObject>), "TWeakkObjectPtr should have same memory layout as FWeakObjectPtr");

	const FWeakObjectPtr& WeakPtr = (const FWeakObjectPtr&)Value;
	return WriteWeakObjectReference(WeakPtr);
}

template<typename TObject>
FDcResult FDcWriter::WriteLazyObjectPtr(const TLazyObjectPtr<TObject>& Value)
{
	//	LazyPtr is missing a copy constructor, can only do assign
	FLazyObjectPtr LazyPtr;
	LazyPtr = Value.GetUniqueID();
	return WriteLazyObjectReference(LazyPtr);
}

template<typename TObject>
FDcResult FDcWriter::WriteSoftObjectPtr(const TSoftObjectPtr<TObject>& Value)
{
	FSoftObjectPtr SoftPtr(Value.GetUniqueID());
	return WriteSoftObjectReference(SoftPtr);
}

