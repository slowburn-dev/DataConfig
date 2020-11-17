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

bool IsScalarProperty(UField* Property)
{
	check(Property);
	bool bIsCompound = Property->IsA<UStructProperty>()
		|| Property->IsA<UObjectProperty>()
		|| Property->IsA<UMapProperty>()
		|| Property->IsA<UArrayProperty>();

	return !bIsCompound;
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
	if (Property->IsA<UNameProperty>()) return EDcDataEntry::Name;
	if (Property->IsA<UStrProperty>()) return EDcDataEntry::String;

	if (Property->IsA<UInt8Property>()) return EDcDataEntry::Int8;
	if (Property->IsA<UInt16Property>()) return EDcDataEntry::Int16;
	if (Property->IsA<UIntProperty>()) return EDcDataEntry::Int32;
	if (Property->IsA<UInt64Property>()) return EDcDataEntry::Int64;

	if (Property->IsA<UByteProperty>()) return EDcDataEntry::UInt8;
	if (Property->IsA<UUInt16Property>()) return EDcDataEntry::UInt16;
	if (Property->IsA<UUInt32Property>()) return EDcDataEntry::UInt32;
	if (Property->IsA<UUInt64Property>()) return EDcDataEntry::UInt64;

	if (Property->IsA<UFloatProperty>()) return EDcDataEntry::Float;
	if (Property->IsA<UDoubleProperty>()) return EDcDataEntry::Double;

	if (Property->IsA<UStructProperty>()) return EDcDataEntry::StructRoot;
	if (Property->IsA<UObjectProperty>()) return EDcDataEntry::ClassRoot;
	if (Property->IsA<UMapProperty>()) return EDcDataEntry::MapRoot;
	if (Property->IsA<UArrayProperty>()) return EDcDataEntry::ArrayRoot;

	checkNoEntry();
	return EDcDataEntry::Ended;
}

FString GetFormatPropertyName(UField* Property)
{
	check(Property);
	//	TODO primitive types actually can use `GetCPPType`
	if (Property->IsA<UBoolProperty>()) return TEXT("bool");
	if (Property->IsA<UNameProperty>()) return TEXT("FName");
	if (Property->IsA<UStrProperty>()) return TEXT("FString");

	if (Property->IsA<UInt8Property>()) return TEXT("int8");
	if (Property->IsA<UInt16Property>()) return TEXT("int16");
	if (Property->IsA<UIntProperty>()) return TEXT("int32");
	if (Property->IsA<UInt64Property>()) return TEXT("int64");

	if (Property->IsA<UByteProperty>()) return TEXT("uint8");
	if (Property->IsA<UUInt16Property>()) return TEXT("uint16");
	if (Property->IsA<UUInt32Property>()) return TEXT("uint32");
	if (Property->IsA<UUInt64Property>()) return TEXT("uint64");

	if (Property->IsA<UFloatProperty>()) return TEXT("float");
	if (Property->IsA<UDoubleProperty>()) return TEXT("double");

	if (Property->IsA<UStructProperty>()
		|| Property->IsA<UScriptStruct>())
	{
		return FString::Printf(TEXT("F%s"), *Property->GetName());
	}

	if (Property->IsA<UObjectProperty>()
		|| Property->IsA<UClass>())
	{
		return FString::Printf(TEXT("U%s"), *Property->GetName());
	}

	if (UMapProperty* MapProperty = Cast<UMapProperty>(Property))
	{
		return FString::Printf(TEXT("TMap<%s, %s>"), 
			*GetFormatPropertyName(MapProperty->KeyProp),
			*GetFormatPropertyName(MapProperty->ValueProp)
		);
	}
	if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		return FString::Printf(TEXT("TArray<%s>"), 
			*GetFormatPropertyName(ArrayProperty->Inner)
		);
	}
	checkNoEntry();

	return FString();
}

void DcPropertyHighlight::FormatNil(TArray<FString>& OutSegments, EFormatSeg SegType)
{
	OutSegments.Add(TEXT("<nil>"));
}

void DcPropertyHighlight::FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, UObject* ClassObject, UClass* Class, UProperty* Property)
{
	OutSegments.Add(FString::Printf(TEXT("(U%s)%s"),
		*GetFormatPropertyName(Class),
		*(ClassObject ? ClassObject->GetName() : TEXT("<null>"))
	));

	if (Property != nullptr
		&& (SegType == EFormatSeg::Last || IsScalarProperty(Property)))
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*GetFormatPropertyName(Property),
			*Property->GetName()));
	}
}

void DcPropertyHighlight::FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, FName StructName, UScriptStruct* StructClass, UProperty* Property)
{
	OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
		*GetFormatPropertyName(StructClass),
		*StructName.ToString()
	));

	if (Property != nullptr
		&& (SegType == EFormatSeg::Last || IsScalarProperty(Property)))
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"), 
			*GetFormatPropertyName(Property),
			*Property->GetName()));
	}
}

void DcPropertyHighlight::FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, UMapProperty* MapProperty, uint16 Index, bool bIsKeyOrValue)
{
	check(MapProperty);
	FString Seg = FString::Printf(TEXT("(TMap<%s, %s>)%s"),
		*GetFormatPropertyName(MapProperty->KeyProp),
		*GetFormatPropertyName(MapProperty->ValueProp),
		*MapProperty->GetName()
	);

	if (bIsKeyOrValue)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, UArrayProperty* ArrayProperty, uint16 Index, bool bIsItem)
{
	check(ArrayProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*GetFormatPropertyName(ArrayProperty),
		*ArrayProperty->Inner->GetName()
	);

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}
