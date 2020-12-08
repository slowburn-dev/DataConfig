#include "DataConfig/Property/DcPropertyUtils.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"

bool IsEffectiveProperty(FProperty* Property)
{
	check(Property);
	return Property->IsA<FBoolProperty>()
		|| Property->IsA<FNumericProperty>()
		|| Property->IsA<FStrProperty>()
		|| Property->IsA<FNameProperty>()
		|| Property->IsA<FTextProperty>()
		|| Property->IsA<FEnumProperty>()
		|| Property->IsA<FStructProperty>()
		|| Property->IsA<FClassProperty>()
		|| Property->IsA<FObjectProperty>()
		|| Property->IsA<FMapProperty>()
		|| Property->IsA<FArrayProperty>()
		|| Property->IsA<FSetProperty>()
		|| Property->IsA<FWeakObjectProperty>()
		|| Property->IsA<FLazyObjectProperty>()
		|| Property->IsA<FSoftObjectProperty>()
		|| Property->IsA<FSoftClassProperty>()
		|| Property->IsA<FInterfaceProperty>()
		|| Property->IsA<FDelegateProperty>()
		|| Property->IsA<FMulticastInlineDelegateProperty>()
		|| Property->IsA<FMulticastSparseDelegateProperty>()
		;
}

bool IsScalarProperty(UField* Property)
{
	check(Property);
	bool bIsCompound = Property->IsA<FStructProperty>()
		|| Property->IsA<FObjectProperty>()
		|| Property->IsA<FMapProperty>()
		|| Property->IsA<FArrayProperty>()
		|| Property->IsA<FSetProperty>();

	return !bIsCompound;
}

size_t CountEffectiveProperties(UStruct* Struct)
{
	check(Struct);
	size_t EffectiveCount = 0;
	for (FProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (IsEffectiveProperty(Property))
		{
			++EffectiveCount;
		}
	}

	return EffectiveCount;
}

FProperty* NextEffectiveProperty(FProperty* Property)
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

FProperty* FirstEffectiveProperty(FProperty* Property)
{
	if (Property == nullptr)
		return nullptr;

	return  IsEffectiveProperty(Property)
		? Property
		: NextEffectiveProperty(Property);
}

FProperty* NextPropertyByName(FProperty* InProperty, const FName& Name)
{
	for (FProperty* Property = InProperty; Property; Property = Property->PropertyLinkNext)
	{
		if (Property->GetFName() == Name
			&& IsEffectiveProperty(Property))
		{
			return Property;
		}
	}

	return nullptr;
}

//	TODO [JUMPTABLE] use a jump table here rather than IsA
//					 as the inheritance isn't even needed
EDcDataEntry PropertyToDataEntry(UField* Property)
{
	check(Property)
	if (Property->IsA<FBoolProperty>()) return EDcDataEntry::Bool;
	if (Property->IsA<FNameProperty>()) return EDcDataEntry::Name;
	if (Property->IsA<FStrProperty>()) return EDcDataEntry::String;
	if (Property->IsA<FTextProperty>()) return EDcDataEntry::Text;
	if (Property->IsA<FEnumProperty>()) return EDcDataEntry::Enum;

	if (Property->IsA<FInt8Property>()) return EDcDataEntry::Int8;
	if (Property->IsA<FInt16Property>()) return EDcDataEntry::Int16;
	if (Property->IsA<FIntProperty>()) return EDcDataEntry::Int32;
	if (Property->IsA<FInt64Property>()) return EDcDataEntry::Int64;

	if (Property->IsA<FByteProperty>()) return EDcDataEntry::UInt8;
	if (Property->IsA<FUInt16Property>()) return EDcDataEntry::UInt16;
	if (Property->IsA<FUInt32Property>()) return EDcDataEntry::UInt32;
	if (Property->IsA<FUInt64Property>()) return EDcDataEntry::UInt64;

	if (Property->IsA<FFloatProperty>()) return EDcDataEntry::Float;
	if (Property->IsA<FDoubleProperty>()) return EDcDataEntry::Double;

	{
		//	order significant
		if (Property->IsA<FClassProperty>()) return EDcDataEntry::ClassReference;
		if (Property->IsA<FStructProperty>()) return EDcDataEntry::StructRoot;
	}

	if (Property->IsA<FWeakObjectProperty>()) return EDcDataEntry::WeakObjectReference;
	if (Property->IsA<FLazyObjectProperty>()) return EDcDataEntry::LazyObjectReference;

	{
		//	order significant
		if (Property->IsA<FSoftClassProperty>()) return EDcDataEntry::SoftClassReference;
		if (Property->IsA<FSoftObjectProperty>()) return EDcDataEntry::SoftObjectReference;
		if (Property->IsA<FInterfaceProperty>()) return EDcDataEntry::InterfaceReference;
		if (Property->IsA<FObjectProperty>()) return EDcDataEntry::ClassRoot;
	}

	if (Property->IsA<FDelegateProperty>()) return EDcDataEntry::Delegate;
	if (Property->IsA<FMulticastInlineDelegateProperty>()) return EDcDataEntry::MulticastInlineDelegate;
	if (Property->IsA<FMulticastSparseDelegateProperty>()) return EDcDataEntry::MulticastSparseDelegate;

	if (Property->IsA<FMapProperty>()) return EDcDataEntry::MapRoot;
	if (Property->IsA<FArrayProperty>()) return EDcDataEntry::ArrayRoot;
	if (Property->IsA<FSetProperty>()) return EDcDataEntry::SetRoot;



	checkNoEntry();
	return EDcDataEntry::Ended;
}

FString GetFormatPropertyTypeName(UField* Property)
{
	check(Property);
	//	TODO primitive types actually can use `GetCPPType`
	//	ref: GetCPPTypeCustom
	if (Property->IsA<FBoolProperty>()) return TEXT("bool");
	if (Property->IsA<FNameProperty>()) return TEXT("FName");
	if (Property->IsA<FTextProperty>()) return TEXT("FText");
	if (Property->IsA<FStrProperty>()) return TEXT("FString");

	if (Property->IsA<FInt8Property>()) return TEXT("int8");
	if (Property->IsA<FInt16Property>()) return TEXT("int16");
	if (Property->IsA<FIntProperty>()) return TEXT("int32");
	if (Property->IsA<FInt64Property>()) return TEXT("int64");

	if (Property->IsA<FByteProperty>()) return TEXT("uint8");
	if (Property->IsA<FUInt16Property>()) return TEXT("uint16");
	if (Property->IsA<FUInt32Property>()) return TEXT("uint32");
	if (Property->IsA<FUInt64Property>()) return TEXT("uint64");

	if (Property->IsA<FFloatProperty>()) return TEXT("float");
	if (Property->IsA<FDoubleProperty>()) return TEXT("double");

	if (Property->IsA<FDelegateProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);
	if (Property->IsA<FMulticastInlineDelegateProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);
	if (Property->IsA<FMulticastSparseDelegateProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);

	if (FEnumProperty* EnumProperty = Cast<FEnumProperty>(Property))
	{
		return FString::Printf(TEXT("E%s"), *EnumProperty->GetEnum()->GetName());
	}

	if (UScriptStruct* Struct = Cast<UScriptStruct>(Property))
	{
		return FString::Printf(TEXT("F%s"), *Struct->GetName());
	}

	if (FStructProperty* StructField = Cast<FStructProperty>(Property))
	{
		return FString::Printf(TEXT("F%s"), *StructField->Struct->GetName());
	}

	{
		//	order significant
		if (UClass* Class = Cast<UClass>(Property))
		{
			return FString::Printf(TEXT("U%s"), *Class->GetName());
		}

		if (FObjectProperty* ObjField = Cast<FObjectProperty>(Property))
		{
			return FString::Printf(TEXT("U%s"), *ObjField->PropertyClass->GetName());
		}
	}

	if (FMapProperty* MapProperty = Cast<FMapProperty>(Property))
	{
		return FString::Printf(TEXT("TMap<%s, %s>"),
			*GetFormatPropertyTypeName(MapProperty->KeyProp),
			*GetFormatPropertyTypeName(MapProperty->ValueProp)
		);
	}
	if (FArrayProperty* ArrayProperty = Cast<FArrayProperty>(Property))
	{
		return FString::Printf(TEXT("TArray<%s>"),
			*GetFormatPropertyTypeName(ArrayProperty->Inner)
		);
	}

	if (FSetProperty* SetProperty = Cast<FSetProperty>(Property))
	{
		return FString::Printf(TEXT("TSet<%s>"),
			*GetFormatPropertyTypeName(SetProperty->ElementProp)
			);
	}

	if (FWeakObjectProperty* WeakProperty = Cast<FWeakObjectProperty>(Property))
	{
		return FString::Printf(TEXT("TWeakObjectPtr<%s>"),
			*GetFormatPropertyTypeName(WeakProperty->PropertyClass)
		);
	}

	if (FLazyObjectProperty* LazyProperty = Cast<FLazyObjectProperty>(Property))
	{
		return FString::Printf(TEXT("TLazyObjectPtr<%s>"),
			*GetFormatPropertyTypeName(LazyProperty->PropertyClass)
		);
	}

	{
		//	order significant
		if (FSoftClassProperty* SoftProperty = Cast<FSoftClassProperty>(Property))
		{
			return FString::Printf(TEXT("TSoftClassPtr<%s>"),
				*GetFormatPropertyTypeName(SoftProperty->MetaClass)
			);
		}

		if (FSoftObjectProperty* SoftProperty = Cast<FSoftObjectProperty>(Property))
		{
			return FString::Printf(TEXT("TSoftObjectPtr<%s>"),
				*GetFormatPropertyTypeName(SoftProperty->PropertyClass)
			);
		}
	}

	if (FInterfaceProperty* InterfaceProperty = Cast<FInterfaceProperty>(Property))
	{
		return FString::Printf(TEXT("TScriptInterface<%s>"),
			*GetFormatPropertyTypeName(InterfaceProperty->InterfaceClass)
		);
	}

	checkNoEntry();

	return FString();
}

void DcPropertyHighlight::FormatNil(TArray<FString>& OutSegments, EFormatSeg SegType)
{
	OutSegments.Add(TEXT("<nil>"));
}

void DcPropertyHighlight::FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ObjectName, UClass* Class, FProperty* Property)
{
	if (SegType != EFormatSeg::ParentIsContainer)
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*GetFormatPropertyTypeName(Class),
			*DcTypeUtils::SafeNameToString(ObjectName)
		));
	}

	if (Property != nullptr
		&& (SegType == EFormatSeg::Last || IsScalarProperty(Property)))
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*GetFormatPropertyTypeName(Property),
			*Property->GetName()));
	}
}

void DcPropertyHighlight::FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& StructName, UScriptStruct* StructClass, FProperty* Property)
{
	if (SegType != EFormatSeg::ParentIsContainer)
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*GetFormatPropertyTypeName(StructClass),
			*DcTypeUtils::SafeNameToString(StructName)
		));
	}

	if (Property != nullptr
		&& (SegType == EFormatSeg::Last || IsScalarProperty(Property)))
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*GetFormatPropertyTypeName(Property),
			*Property->GetName()));
	}
}

void DcPropertyHighlight::FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, FMapProperty* MapProperty, uint16 Index, bool bIsKeyOrValue)
{
	check(MapProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*GetFormatPropertyTypeName(MapProperty),
		*MapProperty->GetName()
	);

	if (bIsKeyOrValue)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, FArrayProperty* ArrayProperty, uint16 Index, bool bIsItem)
{
	check(ArrayProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*GetFormatPropertyTypeName(ArrayProperty),
		*ArrayProperty->Inner->GetName()
	);

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatSet(TArray<FString>& OutSegments, EFormatSeg SegType, FSetProperty* SetProperty, uint16 Index, bool bIsItem)
{
	check(SetProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*GetFormatPropertyTypeName(SetProperty),
		*SetProperty->ElementProp->GetName()
	);

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}
