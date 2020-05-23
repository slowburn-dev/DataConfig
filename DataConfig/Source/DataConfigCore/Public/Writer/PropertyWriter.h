#pragma once

#include "CoreMinimal.h"
#include "PropertyCommon/PropertyDatum.h"
#include "Writer/Writer.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPropertyWriter : public FWriter, private FNoncopyable
{
	FPropertyWriter();
	FPropertyWriter(FPropertyDatum Datum);

	FResult Peek(EDataEntry Next) override;

	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;

	FResult WriteStructRoot(const FName& Name) override;
	FResult WriteStructEnd(const FName& Name) override;

	struct FPropertyState
	{
		FPropertyState();

		using ImplStorageType = TAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<8>> States;
};



}	// namespace DataConfig




