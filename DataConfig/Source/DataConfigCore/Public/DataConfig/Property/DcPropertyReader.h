#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Reader/DcReader.h"

class UProperty;

namespace DataConfig
{

struct DATACONFIGCORE_API FDcPropertyReader : public FDcReader, private FNoncopyable
{
	FDcPropertyReader();
	FDcPropertyReader(FDcPropertyDatum Datum);

	EDataEntry Peek() override;

	FDcResult ReadNil(FContextStorage* CtxPtr) override;
	FDcResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FDcResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FDcResult ReadMapEnd(FContextStorage* CtxPtr) override;
	FDcResult ReadArrayRoot(FContextStorage* CtxPtr) override;
	FDcResult ReadArrayEnd(FContextStorage* CtxPtr) override;
	FDcResult ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) override;

	struct FPropertyState
	{
		using ImplStorageType = TDcAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<8>> States;



};

template<> struct TIsPODType<FDcPropertyReader::FPropertyState> { enum { Value = true }; };

} // namespace DataConfig

