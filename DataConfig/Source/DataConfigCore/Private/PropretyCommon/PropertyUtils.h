#pragma once

class UProperty;
class UStruct;

namespace DataConfig
{

bool IsEffectiveProperty(UProperty* Property);

size_t CountEffectiveProperties(UStruct* Struct);

UProperty* NextEffectiveProperty(UProperty* Property);
UProperty* FirstEffectiveProperty(UProperty* Property);


} // namespace DataConfig
