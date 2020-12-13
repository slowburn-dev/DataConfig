#include "DataConfig/Property/DcPropertyUtils.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"

namespace DcPropertyUtils
{

bool IsEffectiveProperty(FProperty* Property)
{
	check(Property);
#if DC_BUILD_DEBUG
	//	this is for handling cases that UE4 added new property and we're not supporting it yet
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
#else
	return true;
#endif
}

bool IsScalarProperty(FField* Property)
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
EDcDataEntry PropertyToDataEntry(FField* Property)
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

EDcDataEntry PropertyToDataEntry(const FFieldVariant& Field)
{
	check(Field.IsValid());
	if (Field.IsUObject())
	{
		UObject* Obj = Field.ToUObjectUnsafe();
		if (Obj->IsA<UScriptStruct>())
		{
			return EDcDataEntry::StructRoot;
		}
		else if (Obj->IsA<UClass>())
		{
			return EDcDataEntry::ClassRoot;
		}
		else
		{
			checkNoEntry();
			return EDcDataEntry::Ended;
		}
	}
	else
	{
		return PropertyToDataEntry(Field.ToFieldUnsafe());
	}
}

FString GetFormatPropertyTypeName(FField* Property)
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

	if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		return FString::Printf(TEXT("E%s"), *EnumProperty->GetEnum()->GetName());
	}

	if (FStructProperty* StructField = CastField<FStructProperty>(Property))
	{
		return FString::Printf(TEXT("F%s"), *StructField->Struct->GetName());
	}

	if (FObjectProperty* ObjField = CastField<FObjectProperty>(Property))
	{
		return FString::Printf(TEXT("U%s"), *ObjField->PropertyClass->GetName());
	}

	if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		return FString::Printf(TEXT("TMap<%s, %s>"),
			*GetFormatPropertyTypeName(MapProperty->KeyProp),
			*GetFormatPropertyTypeName(MapProperty->ValueProp)
		);
	}
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		return FString::Printf(TEXT("TArray<%s>"),
			*GetFormatPropertyTypeName(ArrayProperty->Inner)
		);
	}

	if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		return FString::Printf(TEXT("TSet<%s>"),
			*GetFormatPropertyTypeName(SetProperty->ElementProp)
			);
	}

	if (FWeakObjectProperty* WeakProperty = CastField<FWeakObjectProperty>(Property))
	{
		return FString::Printf(TEXT("TWeakObjectPtr<%s>"),
			*GetFormatPropertyTypeName(WeakProperty->PropertyClass)
		);
	}

	if (FLazyObjectProperty* LazyProperty = CastField<FLazyObjectProperty>(Property))
	{
		return FString::Printf(TEXT("TLazyObjectPtr<%s>"),
			*GetFormatPropertyTypeName(LazyProperty->PropertyClass)
		);
	}

	{
		//	order significant
		if (FSoftClassProperty* SoftProperty = CastField<FSoftClassProperty>(Property))
		{
			return FString::Printf(TEXT("TSoftClassPtr<%s>"),
				*GetFormatPropertyTypeName(SoftProperty->MetaClass)
			);
		}

		if (FSoftObjectProperty* SoftProperty = CastField<FSoftObjectProperty>(Property))
		{
			return FString::Printf(TEXT("TSoftObjectPtr<%s>"),
				*GetFormatPropertyTypeName(SoftProperty->PropertyClass)
			);
		}
	}

	if (FInterfaceProperty* InterfaceProperty = CastField<FInterfaceProperty>(Property))
	{
		return FString::Printf(TEXT("TScriptInterface<%s>"),
			*GetFormatPropertyTypeName(InterfaceProperty->InterfaceClass)
		);
	}

	checkNoEntry();
	return FString();
}

FString GetFormatPropertyTypeName(UScriptStruct* Struct)
{
	check(Struct);
	return FString::Printf(TEXT("F%s"), *Struct->GetName());
}

FString GetFormatPropertyTypeName(UClass* Class)
{
	check(Class);
	return FString::Printf(TEXT("U%s"), *Class->GetName());
}

FString GetFormatPropertyTypeName(const FFieldVariant& Field)
{
	check(Field.IsValid());
	if (Field.IsUObject())
	{
		UObject* Obj = Field.ToUObjectUnsafe();
		if (UScriptStruct* Struct = Cast<UScriptStruct>(Obj))
		{
			return GetFormatPropertyTypeName(Struct);
		}
		else if (UClass* Class = Cast<UClass>(Obj))
		{
			return GetFormatPropertyTypeName(Class);
		}
		else
		{
			checkNoEntry();
			return TEXT("<invalid>");
		}
	}
	else
	{
		return GetFormatPropertyTypeName(Field.ToFieldUnsafe());
	}
}

FName GetStructTypeName(FFieldVariant& Property)
{
	if (!Property.IsValid())
	{
		return FName();
	}
	else if (Property.IsA<FStructProperty>())
	{
		return CastFieldChecked<FStructProperty>(Property.ToFieldUnsafe())->Struct->GetFName();
	}
	else if (Property.IsA<UScriptStruct>())
	{
		return CastChecked<UScriptStruct>(Property.ToUObjectUnsafe())->GetFName();
	}
	else
	{
		return FName();
	}
}

}	// namespace DcPropertyUtils
