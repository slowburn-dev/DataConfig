#pragma once

class UProperty;
class UStruct;

namespace DataConfig
{


bool IsEffectiveProperty(UProperty* Property);

size_t CountEffectiveProperties(UStruct* Struct);

UProperty* NextEffectiveProperty(UProperty* Property);
UProperty* FirstEffectiveProperty(UProperty* Property);
UProperty* NextPropertyByName(UProperty* InProperty, const FName& Name);

enum class EDataEntry;
EDataEntry PropertyToDataEntry(UField* Property);

template<typename TState, typename TStorage, typename... TArgs>
TState& Emplace(TStorage* Storage, TArgs&&... Args)
{
	static_assert(sizeof(TState) <= sizeof(TStorage), "storage too small");
	return *(new (Storage) TState(Forward<TArgs>(Args)...));
}

} // namespace DataConfig
