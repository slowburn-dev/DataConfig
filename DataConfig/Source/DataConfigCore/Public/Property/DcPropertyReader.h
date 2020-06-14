#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Property/DcPropertyDatum.h"
#include "Reader/DcReader.h"

class UProperty;

namespace DataConfig
{

struct DATACONFIGCORE_API FPropertyReader : public FReader, private FNoncopyable
{
	FPropertyReader();
	FPropertyReader(FPropertyDatum Datum);

	EDataEntry Peek() override;

	FResult ReadNil(FContextStorage* CtxPtr) override;
	FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FResult ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FResult ReadMapEnd(FContextStorage* CtxPtr) override;
	FResult ReadArrayRoot(FContextStorage* CtxPtr) override;
	FResult ReadArrayEnd(FContextStorage* CtxPtr) override;
	FResult ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) override;

	struct FPropertyState
	{
		using ImplStorageType = TAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<8>> States;



};

template<> struct TIsPODType<FPropertyReader::FPropertyState> { enum { Value = true }; };

} // namespace DataConfig

