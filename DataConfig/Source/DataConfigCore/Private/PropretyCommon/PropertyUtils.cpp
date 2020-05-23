#include "PropertyUtils.h"
#include "UObject/UnrealType.h"

namespace DataConfig
{

bool IsEffectiveProperty(UProperty* Property)
{
	check(Property);
	return Property->IsA<UBoolProperty>()
		|| Property->IsA<UFloatProperty>()
		|| Property->IsA<UDoubleProperty>()
		|| Property->IsA<UIntProperty>()
		|| Property->IsA<UUInt32Property>()
		|| Property->IsA<UStrProperty>()
		|| Property->IsA<UNameProperty>()
		|| Property->IsA<UStructProperty>()
		|| Property->IsA<UObjectProperty>()
		|| Property->IsA<UMapProperty>()
		|| Property->IsA<UArrayProperty>();
}

size_t CountEffectiveProperties(UStruct* Struct)
{
	check(Struct);
	size_t EffectiveCount = 0;
	for (UProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (IsEffectiveProperty(Property))
		{
			++EffectiveCount;
		}
	}

	return EffectiveCount;
}

UProperty* NextEffectiveProperty(UProperty* Property) 
{
	while (true)
	{
		if (Property == nullptr)
			return nullptr;

		Property = Property->PropertyLinkNext;

		if (Property == nullptr)
			return nullptr;

		if (IsEffectiveProperty(Property))
			return Property;
	}

	checkNoEntry();
	return nullptr;
}

UProperty* FirstEffectiveProperty(UProperty* Property)
{
	if (Property == nullptr)
		return nullptr;

	return  IsEffectiveProperty(Property)
		? Property
		: NextEffectiveProperty(Property);
}

EDataEntry PropertyToDataEntry(UProperty* Property)
{
	check(Property)
	if (Property->IsA<UBoolProperty>()) return EDataEntry::Bool;
	if (Property->IsA<UFloatProperty>()) return EDataEntry::Float;
	if (Property->IsA<UDoubleProperty>()) return EDataEntry::Double;
	if (Property->IsA<UIntProperty>()) return EDataEntry::Int;
	if (Property->IsA<UUInt32Property>()) return EDataEntry::UInt32;
	if (Property->IsA<UStrProperty>()) return EDataEntry::String;
	if (Property->IsA<UNameProperty>()) return EDataEntry::Name;
	if (Property->IsA<UStructProperty>()) return EDataEntry::StructRoot;
	if (Property->IsA<UObjectProperty>()) return EDataEntry::ClassRoot;
	if (Property->IsA<UMapProperty>()) return EDataEntry::MapRoot;
	if (Property->IsA<UArrayProperty>()) return EDataEntry::ArrayRoot;
	checkNoEntry();
	return EDataEntry::Ended;
}

} // namespace DataConfig
