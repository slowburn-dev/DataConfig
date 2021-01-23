#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

class FProperty;

struct DATACONFIGCORE_API FDcPropertyReader : public FDcReader, private FNoncopyable
{
	FDcPropertyReader();
	FDcPropertyReader(FDcPropertyDatum Datum);

	bool Coercion(EDcDataEntry ToEntry) override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;

	FDcResult ReadNil() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadText(FText* OutPtr) override;
	FDcResult ReadEnum(FDcEnumData* OutPtr) override;

	FDcResult ReadStructRoot(FDcStructStat* OutStructPtr) override;
	FDcResult ReadStructEnd(FDcStructStat* OutStructPtr) override;

	FDcResult ReadClassRoot(FDcClassStat* OutClassPtr) override;
	FDcResult ReadClassEnd(FDcClassStat* OutClassPtr) override;

	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;

	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;

	FDcResult ReadSetRoot() override;
	FDcResult ReadSetEnd() override;

	FDcResult ReadObjectReference(UObject** OutPtr) override;
	FDcResult ReadClassReference(UClass** OutPtr) override;

	FDcResult ReadWeakObjectReference(FWeakObjectPtr* OutPtr) override;
	FDcResult ReadLazyObjectReference(FLazyObjectPtr* OutPtr) override;
	FDcResult ReadSoftObjectReference(FSoftObjectPath* OutPtr) override;
	FDcResult ReadSoftClassReference(FSoftClassPath* OutPtr) override;
	FDcResult ReadInterfaceReference(FScriptInterface* OutPtr) override;

	FDcResult ReadFieldPath(FFieldPath* OutPtr) override;
	FDcResult ReadDelegate(FScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastSparseDelegate(FMulticastScriptDelegate* OutPtr) override;

	FDcResult ReadInt8(int8* OutPtr) override;
	FDcResult ReadInt16(int16* OutPtr) override;
	FDcResult ReadInt32(int32* OutPtr) override;
	FDcResult ReadInt64(int64* OutPtr) override;

	FDcResult ReadUInt8(uint8* OutPtr) override;
	FDcResult ReadUInt16(uint16* OutPtr) override;
	FDcResult ReadUInt32(uint32* OutPtr) override;
	FDcResult ReadUInt64(uint64* OutPtr) override;

	FDcResult ReadFloat(float* OutPtr) override;
	FDcResult ReadDouble(double* OutPtr) override;
	FDcResult ReadBlob(FDcBlobViewData* OutPtr) override;

	///	try skip read at current position
	FDcResult SkipRead();
	///	peek next write property
	FDcResult PeekReadProperty(FFieldVariant* OutProperty);
	///	manual reading
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);

	FDcResult SetConfig(FDcPropertyConfig InConfig);
	FDcPropertyConfig Config;

	struct FPropertyState
	{
		using ImplStorageType = TAlignedBytes<64, MIN_ALIGNMENT>;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<4>> States;

	FDcDiagnosticHighlight FormatHighlight();
	void FormatDiagnostic(FDcDiagnostic& Diag) override;

};

template<> struct TIsPODType<FDcPropertyReader::FPropertyState> { enum { Value = true }; };

