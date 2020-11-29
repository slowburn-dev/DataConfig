#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

class UProperty;

struct DATACONFIGCORE_API FDcPropertyReader : public FDcReader, private FNoncopyable
{
	FDcPropertyReader();
	FDcPropertyReader(FDcPropertyDatum Datum);

	FDcResult ReadNext(EDcDataEntry* OutPtr) override;

	FDcResult ReadNil() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadText(FText* OutPtr) override;
	FDcResult ReadEnum(FDcEnumData* OutPtr) override;

	FDcResult ReadStructRoot(FName* OutNamePtr) override;
	FDcResult ReadStructEnd(FName* OutNamePtr) override;

	FDcResult ReadClassRoot(FDcObjectPropertyStat* OutClassPtr) override;
	FDcResult ReadClassEnd(FDcObjectPropertyStat* OutClassPtr) override;

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

	FDcResult ReadDelegate(FScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastSparseDelegate(FSparseDelegate* OutPtr) override;

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

	struct FPropertyState
	{
		using ImplStorageType = TDcAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<4>> States;

	FDcDiagnosticHighlight FormatHighlight();
	void FormatDiagnostic(FDcDiagnostic& Diag) override;

};

template<> struct TIsPODType<FDcPropertyReader::FPropertyState> { enum { Value = true }; };

