#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Reader/DcReader.h"

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
	FDcResult ReadStructRoot(FName* OutNamePtr) override;
	FDcResult ReadStructEnd(FName* OutNamePtr) override;
	FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr) override;
	FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr) override;
	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;
	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;
	FDcResult ReadReference(UObject** OutPtr) override;

	struct FPropertyState
	{
		using ImplStorageType = TDcAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<8>> States;

};

template<> struct TIsPODType<FDcPropertyReader::FPropertyState> { enum { Value = true }; };


