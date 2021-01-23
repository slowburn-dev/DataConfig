#pragma once

#include "CoreMinimal.h"

template<typename TState, typename TStorage, typename... TArgs>
TState& Emplace(TStorage* Storage, TArgs&&... Args)
{
	static_assert(sizeof(TState) <= sizeof(TStorage), "storage too small");
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
	void FormatScalar(TArray<FString>& OutSegments, EFormatSeg SegType, FField* Property);
	void FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ObjectName, UClass* Class, FProperty* Property);
	void FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& StructName, UScriptStruct* StructClass, FProperty* Property);
	void FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, FMapProperty* MapProperty, uint16 Index, bool bIsKeyOrValue);;
	void FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, FArrayProperty* ArrayProperty, uint16 Index, bool bIsItem);
	void FormatSet(TArray<FString>& OutSegments, EFormatSeg SegType, FSetProperty* SetProperty, uint16 Index, bool bIsItem);

} // namespace DcPropertyHighlight





