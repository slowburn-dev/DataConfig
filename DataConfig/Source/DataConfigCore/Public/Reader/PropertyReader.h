#pragma once

#include "CoreMinimal.h"
#include "DataConfigTypes.h"
#include "PropertyCommon/PropertyDatum.h"
#include "Reader/Reader.h"

class UProperty;

namespace DataConfig
{

struct DATACONFIGCORE_API FPropertyReader : public FReader, private FNoncopyable
{
	FPropertyReader();
	FPropertyReader(FPropertyDatum Datum);

	EDataEntry Peek() override;

	FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FResult ReadMapEnd(FContextStorage* CtxPtr) override;

	struct FPropertyState
	{
		using ImplStorageType = TAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<8>> States;
};

template<> struct TIsPODType<FPropertyReader::FPropertyState> { enum { Value = true }; };

} // namespace DataConfig

