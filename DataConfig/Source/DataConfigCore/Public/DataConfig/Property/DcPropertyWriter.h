#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

struct DATACONFIGCORE_API FDcPropertyWriter : public FDcWriter, private FNoncopyable
{
	FDcPropertyWriter();
	FDcPropertyWriter(FDcPropertyDatum Datum);

	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;

	FDcResult WriteNil() override;
	FDcResult WriteBool(bool Value) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteString(const FString& Value) override;
	FDcResult WriteText(const FText& Value) override;
	FDcResult WriteEnum(const FDcEnumData& Value) override;

	FDcResult WriteStructRoot(const FName& Name) override;
	FDcResult WriteStructEnd(const FName& Name) override;

	FDcResult WriteClassRoot(const FDcObjectPropertyStat& Class) override;
	FDcResult WriteClassEnd(const FDcObjectPropertyStat& Class) override;

	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;

	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;

	FDcResult WriteSetRoot() override;
	FDcResult WriteSetEnd() override;

	FDcResult WriteObjectReference(const UObject* Value) override;
	FDcResult WriteClassReference(const UClass* Value) override;

	FDcResult WriteWeakObjectReference(const FWeakObjectPtr& Value) override;
	FDcResult WriteLazyObjectReference(const FLazyObjectPtr& Value) override;
	FDcResult WriteSoftObjectReference(const FSoftObjectPath& Value) override;
	FDcResult WriteSoftClassReference(const FSoftClassPath& Value) override;
	FDcResult WriteInterfaceReference(const FScriptInterface& Value) override;

	FDcResult WriteDelegate(const FScriptDelegate& Value) override;
	FDcResult WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value) override;
	FDcResult WriteMulticastSparseDelegate(const FSparseDelegate& Value) override;

	FDcResult WriteInt8(const int8& Value) override;
	FDcResult WriteInt16(const int16& Value) override;
	FDcResult WriteInt32(const int32& Value) override;
	FDcResult WriteInt64(const int64& Value) override;

	FDcResult WriteUInt8(const uint8& Value) override;
	FDcResult WriteUInt16(const uint16& Value) override;
	FDcResult WriteUInt32(const uint32& Value) override;
	FDcResult WriteUInt64(const uint64& Value) override;

	FDcResult WriteFloat(const float& Value) override;
	FDcResult WriteDouble(const double& Value) override;
	FDcResult WriteBlob(const FDcBlobViewData& Value) override;

	//	skip current write at current position
	FDcResult SkipWrite();
	//	get the next write property
	FDcResult PeekWriteProperty(UField** OutProperty);
	//	manual writing
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);

	//	manual writing supporting
	void PushTopClassPropertyState(FDcPropertyDatum& Datum);
	void PushTopStructPropertyState(FDcPropertyDatum& Datum, const FName& StructName);

	struct FPropertyState
	{
		using ImplStorageType = TDcAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<4>> States;

	FDcDiagnosticHighlight FormatHighlight();
	void FormatDiagnostic(FDcDiagnostic& Diag) override;
};



