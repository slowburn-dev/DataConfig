#pragma once

#include "CoreMinimal.h"

extern FName DC_TRANSIENT_ARRAY;
extern FName DC_TRANSIENT_SET;
extern FName DC_TRANSIENT_MAP;

template<typename TState, typename TStorage, typename... TArgs>
TState& Emplace(TStorage* Storage, TArgs&&... Args)
{
	static_assert(sizeof(TState) <= sizeof(TStorage), "storage too small");
	static_assert(alignof(TStorage) >= alignof(TState), "align fail");
	static_assert(sizeof(TStorage) % alignof(TState) == 0, "align fail");
	return *(new (Storage) TState(Forward<TArgs>(Args)...));
}

namespace DcPropertyHighlight
{
	enum class EFormatSeg
	{
		Normal,
		ParentIsContainer,
		Last,
	};

	void FormatNil(TArray<FString>& OutSegments, EFormatSeg SegType);
	void FormatScalar(TArray<FString>& OutSegments, EFormatSeg SegType, FProperty* Property, int Index, bool bIsItem);
	void FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ObjectName, UClass* Class, FProperty* Property);
	void FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& StructName, UScriptStruct* StructClass, FProperty* Property);
	void FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& MapName, FProperty* KeyProperty, FProperty* ValueProperty, int Index, bool bIsKeyOrValue);
	void FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ArrayName, FProperty* InnerProperty, int Index, bool bIsItem);
	void FormatSet(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& SetName, FProperty* ElementProperty, int Index, bool bIsItem);

} // namespace DcPropertyHighlight





