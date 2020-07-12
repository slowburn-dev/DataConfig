#pragma once

#include "CoreMinimal.h"
#include "Property/DcPropertyDatum.h"
#include "Writer/DcWriter.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPropertyWriter : public FWriter, private FNoncopyable
{
	FPropertyWriter();
	FPropertyWriter(FPropertyDatum Datum);

	FResult Peek(EDataEntry Next) override;

	FResult WriteNil() override;
	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;

	FResult WriteStructRoot(const FName& Name) override;
	FResult WriteStructEnd(const FName& Name) override;

	FResult WriteClassRoot(const FClassPropertyStat& Class) override;
	FResult WriteClassEnd(const FClassPropertyStat& Class) override;

	FResult WriteMapRoot() override;
	FResult WriteMapEnd() override;

	FResult WriteArrayRoot() override;
	FResult WriteArrayEnd() override;

	FResult WriteReference(UObject* Value) override;

	//	skip current write at current position
	FResult SkipWrite();
	//	get the next write property
	FResult PeekWriteProperty(UField** OutProperty);
	//	manual writing
	FResult WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum);

	//	manual writing supporting
	FResult PushTopClassProperty(const FClassPropertyStat& Class, FPropertyDatum& Datum);

	struct FPropertyState
	{
		using ImplStorageType = TAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<4>> States;
};



}	// namespace DataConfig




