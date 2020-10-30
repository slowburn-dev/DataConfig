#include "DataConfig/Property/DcPropertyUtils.h"
#include "UObject/UnrealType.h"

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

UProperty* NextPropertyByName(UProperty* InProperty, const FName& Name)
{
	for (UProperty* Property = InProperty; Property; Property = Property->PropertyLinkNext)
	{
		if (Property->GetFName() == Name
			&& IsEffectiveProperty(Property))
		{
			return Property;
		}
	}

	return nullptr;
}

EDcDataEntry PropertyToDataEntry(UField* Property)
{
	check(Property)
	if (Property->IsA<UBoolProperty>()) return EDcDataEntry::Bool;
	if (Property->IsA<UFloatProperty>()) return EDcDataEntry::Float;
	if (Property->IsA<UDoubleProperty>()) return EDcDataEntry::Double;
	if (Property->IsA<UIntProperty>()) return EDcDataEntry::Int32;
	if (Property->IsA<UUInt32Property>()) return EDcDataEntry::UInt32;
	if (Property->IsA<UStrProperty>()) return EDcDataEntry::String;
	if (Property->IsA<UNameProperty>()) return EDcDataEntry::Name;
	if (Property->IsA<UStructProperty>()) return EDcDataEntry::StructRoot;
	if (Property->IsA<UObjectProperty>()) return EDcDataEntry::ClassRoot;
	if (Property->IsA<UMapProperty>()) return EDcDataEntry::MapRoot;
	if (Property->IsA<UArrayProperty>()) return EDcDataEntry::ArrayRoot;
	checkNoEntry();
	return EDcDataEntry::Ended;
}

