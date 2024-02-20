#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"
#include "UObject/PropertyAccessUtil.h"

#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
#include "UObject/PropertyOptional.h"
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

namespace DcPropertyUtils
{

const FName DC_META_SKIP = FName(TEXT("DcSkip"));

bool IsScalarProperty(FField* Property)
{
	check(Property);
	bool bIsCompound = Property->IsA<FStructProperty>()
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
		|| Property->IsA<FOptionalProperty>()
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
		|| Property->IsA<FObjectProperty>()
		|| Property->IsA<FMapProperty>()
		|| Property->IsA<FArrayProperty>()
		|| Property->IsA<FSetProperty>();

	return !bIsCompound;
}

bool IsScalarArray(FField* Property)
{
	return CastFieldChecked<FProperty>(Property)->ArrayDim > 1;
}

bool IsScalarArray(FFieldVariant Property)
{
	if (Property.IsUObject())
		return false;
	else
		return IsScalarArray(Property.ToFieldUnsafe());
}

void VisitAllEffectivePropertyClass(TFunctionRef<void(FFieldClass*)> Visitor)
{
	Visitor(FBoolProperty::StaticClass());

	Visitor(FInt8Property::StaticClass());
	Visitor(FInt16Property::StaticClass());
	Visitor(FIntProperty::StaticClass());
	Visitor(FInt64Property::StaticClass());

	Visitor(FByteProperty::StaticClass());
	Visitor(FUInt16Property::StaticClass());
	Visitor(FUInt32Property::StaticClass());
	Visitor(FUInt64Property::StaticClass());

	Visitor(FFloatProperty::StaticClass());
	Visitor(FDoubleProperty::StaticClass());

	Visitor(FStrProperty::StaticClass());
	Visitor(FNameProperty::StaticClass());
	Visitor(FTextProperty::StaticClass());
	Visitor(FEnumProperty::StaticClass());
	Visitor(FStructProperty::StaticClass());
	Visitor(FClassProperty::StaticClass());
	Visitor(FObjectProperty::StaticClass());

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
	Visitor(FClassPtrProperty::StaticClass());
	Visitor(FObjectPtrProperty::StaticClass());
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3

	Visitor(FMapProperty::StaticClass());
	Visitor(FArrayProperty::StaticClass());
	Visitor(FSetProperty::StaticClass());
	Visitor(FWeakObjectProperty::StaticClass());
	Visitor(FLazyObjectProperty::StaticClass());
	Visitor(FSoftObjectProperty::StaticClass());
	Visitor(FSoftClassProperty::StaticClass());
	Visitor(FInterfaceProperty::StaticClass());
	Visitor(FDelegateProperty::StaticClass());
	Visitor(FFieldPathProperty::StaticClass());
	Visitor(FMulticastInlineDelegateProperty::StaticClass());
	Visitor(FMulticastSparseDelegateProperty::StaticClass());

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	Visitor(FOptionalProperty::StaticClass());
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

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

	return IsEffectiveProperty(Property)
		? Property
		: NextEffectiveProperty(Property);
}

FProperty* FindEffectivePropertyByName(UStruct* Struct, const FName& Name)
{
	FProperty* Property = PropertyAccessUtil::FindPropertyByName(Name, Struct);
	if (Property == nullptr)
		return nullptr;
	
	return IsEffectiveProperty(Property)
		? Property
		: nullptr;
}

FProperty* FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset)
{
	FProperty* Property = Struct->PropertyLink;
	while (true)
	{
		if (Property == nullptr)
			return nullptr;

		if (!IsEffectiveProperty(Property))
			return nullptr;

		if (Property->GetOffset_ForInternal() == Offset)
			return Property;

		Property = Property->PropertyLinkNext;
	}

	return nullptr;
}

FDcResult FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset, FProperty*& OutValue)
{
	OutValue = FindEffectivePropertyByOffset(Struct, Offset);
	if (OutValue == nullptr)
	{
		return DC_FAIL(DcDReadWrite, FindPropertyByOffsetFailed)
			<< Struct->GetFName() << (uint64)(Offset);
	}
	else
	{
		return DcOk();
	}
}

EDcDataEntry PropertyToDataEntry(FField* Property)
{
	check(Property)
	if (Property->IsA<FBoolProperty>()) return EDcDataEntry::Bool;
	if (Property->IsA<FNameProperty>()) return EDcDataEntry::Name;
	if (Property->IsA<FStrProperty>()) return EDcDataEntry::String;
	if (Property->IsA<FTextProperty>()) return EDcDataEntry::Text;
	if (Property->IsA<FEnumProperty>()) return EDcDataEntry::Enum;

	if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		if (NumericProperty->IsEnum())
		{
			return EDcDataEntry::Enum;
		}
		else
		{
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
		}
	}

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

	if (Property->IsA<FFieldPathProperty>()) return EDcDataEntry::FieldPath;
	if (Property->IsA<FDelegateProperty>()) return EDcDataEntry::Delegate;
	if (Property->IsA<FMulticastInlineDelegateProperty>()) return EDcDataEntry::MulticastInlineDelegate;
	if (Property->IsA<FMulticastSparseDelegateProperty>()) return EDcDataEntry::MulticastSparseDelegate;

	if (Property->IsA<FMapProperty>()) return EDcDataEntry::MapRoot;
	if (Property->IsA<FArrayProperty>()) return EDcDataEntry::ArrayRoot;
	if (Property->IsA<FSetProperty>()) return EDcDataEntry::SetRoot;

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	if (Property->IsA<FOptionalProperty>()) return EDcDataEntry::OptionalRoot;
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

	checkNoEntry();
	return EDcDataEntry::Ended;
}

FString FormatArrayTypeName(FProperty* InnerProperty)
{
	return FString::Printf(TEXT("TArray<%s>"),
		*GetFormatPropertyTypeName(InnerProperty)
	);
}

FString FormatSetTypeName(FProperty* InnerProperty)
{
	return FString::Printf(TEXT("TSet<%s>"),
		*GetFormatPropertyTypeName(InnerProperty)
	);
}

FString FormatMapTypeName(FProperty* KeyProperty, FProperty* ValueProperty)
{
	return FString::Printf(TEXT("TMap<%s, %s>"),
		*GetFormatPropertyTypeName(KeyProperty),
		*GetFormatPropertyTypeName(ValueProperty)
	);
}

FString FormatOptionalTypeName(FProperty* ValueProperty)
{
	return FString::Printf(TEXT("TOptional<%s>"),
		*GetFormatPropertyTypeName(ValueProperty)
	);
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

	if (Property->IsA<FFieldPathProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);
	if (Property->IsA<FDelegateProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);
	if (Property->IsA<FMulticastInlineDelegateProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);
	if (Property->IsA<FMulticastSparseDelegateProperty>()) return ((FProperty*)Property)->GetCPPType(nullptr, 0);

	if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		UEnum* Enum = EnumProperty->GetEnum();
		return Enum
			? FString::Printf(TEXT("E%s"), *(EnumProperty->GetEnum()->GetName()))
			: TEXT("<Null Enum>");
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
		return FormatMapTypeName(MapProperty->KeyProp, MapProperty->ValueProp);
	}

	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		return FormatArrayTypeName(ArrayProperty->Inner);
	}

	if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		return FormatSetTypeName(SetProperty->ElementProp);
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

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	if (FOptionalProperty* OptionalProperty = CastField<FOptionalProperty>(Property))
	{
		return FString::Printf(TEXT("TOptional<%s>"),
			*GetFormatPropertyTypeName(OptionalProperty->GetValueProperty())
		);
	}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

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

bool IsSubObjectProperty(FObjectProperty* ObjectProperty)
{
	//	check `UPROPERTY(Instanced)`
	//	note that `UCLASS(DefaultToInstanced)` would auto mark this
	return ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference);
}

bool IsUnsignedProperty(FNumericProperty* NumericProperty)
{
	return NumericProperty->IsA<FByteProperty>()
		|| NumericProperty->IsA<FUInt16Property>()
		|| NumericProperty->IsA<FUInt32Property>()
		|| NumericProperty->IsA<FUInt64Property>();
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

UScriptStruct* TryGetStructClass(FFieldVariant& FieldVariant)
{
	if (FStructProperty* StructProperty = CastFieldVariant<FStructProperty>(FieldVariant))
	{
		return StructProperty->Struct;
	}
	else if (UScriptStruct* StructClass = CastFieldVariant<UScriptStruct>(FieldVariant))
	{
		return StructClass;
	}
	else
	{
		return nullptr;
	}
}

bool IsEnumAndTryUnwrapEnum(const FFieldVariant& Field, UEnum*& OutEnum, FNumericProperty*& OutNumeric)
{
	if (FEnumProperty* EnumProperty = CastFieldVariant<FEnumProperty>(Field))
	{
		OutNumeric = EnumProperty->GetUnderlyingProperty();
		OutEnum = EnumProperty->GetEnum();
		return OutNumeric != nullptr;
	}
	else if (FNumericProperty* NumericProperty = CastFieldVariant<FNumericProperty>(Field))
	{
		if (NumericProperty->IsEnum())
		{
			OutNumeric = NumericProperty;
			OutEnum = OutNumeric->GetIntPropertyEnum();
			return true;
		}
	}

	return false;
}

bool HeuristicIsPointerInvalid(const void* Ptr)
{
	//	It's too easy to run into uninitialized pointers that leads to crash, especially when dealing with FStructs.
	//	So instead we decided to do a heuristic pointer memory pattern check when `DC_BUILD_DEBUG=1`
	//	Worst case of this would be false positive but it's better than a crash, and it's highly likely
	//	not going to happen
	SIZE_T PtrPattern = (SIZE_T)Ptr;
	bool bInvalid;
#if PLATFORM_64BITS
	switch (PtrPattern)
	{
		case 0xCCCC'CCCC'CCCC'CCCC:
		case 0xCDCD'CDCD'CDCC'CDCD:
		case 0xDEAD'BEEF'DEAD'BEAF:
		case 0xFEEE'FEEE'FEEE'FEEE:
		case 0xABAB'ABAB'ABAB'ABAB:
		case 0xFDFD'FDFD'FDFD'FDFD:
			bInvalid = true;
			break;
		default:
			bInvalid = false;
			break;
	}
#else
	switch (PtrPattern)
	{
		case 0xCCCC'CCCC:
		case 0xCDCD'CDCD:
		case 0xDEAD'BEEF:
		case 0xFEEE'FEEE:
		case 0xABAB'ABAB:
		case 0xFDFD'FDFD:
			bInvalid = true;
			break;
		default:
			bInvalid = false;
			break;
	}
#endif
	return bInvalid;
}

FDcResult HeuristicVerifyPointer(const void* Ptr)
{
#if DC_BUILD_DEBUG
	if (HeuristicIsPointerInvalid(Ptr))
	{
		return DC_FAIL(DcDReadWrite, HeuristicInvalidPointer)
			<< FString::Printf(TEXT("0x%p"), Ptr);
	}

	return DcOk();
#else
	return DcOk();
#endif
}

UStruct* TryGetStruct(const FFieldVariant& FieldVariant)
{
	if (FieldVariant.IsA<UScriptStruct>())
	{
		return (UScriptStruct*)FieldVariant.ToUObjectUnsafe();
	}
	else if (FieldVariant.IsA<UClass>())
	{
		return (UClass*)FieldVariant.ToUObjectUnsafe();
	}
	else if (FieldVariant.IsA<FStructProperty>())
	{
		return CastFieldChecked<FStructProperty>(FieldVariant.ToFieldUnsafe())->Struct;
	}
	else if (FieldVariant.IsA<FObjectProperty>())
	{
		return CastFieldChecked<FObjectProperty>(FieldVariant.ToFieldUnsafe())->PropertyClass;
	}
	else
	{
		return nullptr;
	}
}

UStruct* TryGetStruct(const FDcPropertyDatum& Datum)
{
	return TryGetStruct(Datum.Property);
}

const FName DC_TRANSIENT_PROPERTY = FName(TEXT("DcPropertyBuilderTransientProperty"));

FDcPropertyBuilder FDcPropertyBuilder::Make(
	FFieldClass* PropertyClass, 
	const FName InName,
	FFieldVariant InOuter
)
{
	FDcPropertyBuilder Ret;
	Ret.Property = CastFieldChecked<FProperty>(PropertyClass->Construct(InOuter, InName, RF_NoFlags));
	Ret.Property->ArrayDim = 1;
	return Ret;
}

FDcPropertyBuilder& FDcPropertyBuilder::ArrayDim(int InArrayDim)
{
	Property->ArrayDim = InArrayDim;
	return *this;
}

void FDcPropertyBuilder::Link()
{
	FArchive Ar;
	Property->LinkWithoutChangingOffset(Ar);
}

FDcPropertyBuilder FDcPropertyBuilder::Object(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FObjectProperty>(InName, InOuter);
	Ret.As<FObjectProperty>()->SetPropertyClass(InClass);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Class(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FClassProperty>(InName, InOuter);
	Ret.As<FClassProperty>()->SetPropertyClass(UClass::StaticClass());
	Ret.As<FClassProperty>()->SetMetaClass(InClass);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Struct(UScriptStruct* InStruct, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FStructProperty>(InName, InOuter);
	Ret.As<FStructProperty>()->Struct = InStruct;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::SoftObject(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FSoftObjectProperty>(InName, InOuter);
	Ret.As<FSoftObjectProperty>()->SetPropertyClass(InClass);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::SoftClass(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FSoftClassProperty>(InName, InOuter);
	Ret.As<FSoftClassProperty>()->SetPropertyClass(UClass::StaticClass());
	Ret.As<FSoftClassProperty>()->SetMetaClass(InClass);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::LazyObject(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FLazyObjectProperty>(InName, InOuter);
	Ret.As<FLazyObjectProperty>()->SetPropertyClass(InClass);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Delegate(UFunction* InFunction, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FDelegateProperty>(InName, InOuter);
	Ret.As<FDelegateProperty>()->SignatureFunction = InFunction;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::MulticastInlineDelegate(UDelegateFunction* InFunction, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FMulticastInlineDelegateProperty>(InName, InOuter);
	Ret.As<FMulticastInlineDelegateProperty>()->SignatureFunction = InFunction;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::MulticastSparseDelegate(USparseDelegateFunction* InFunction, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FMulticastSparseDelegateProperty>(InName, InOuter);
	Ret.As<FMulticastSparseDelegateProperty>()->SignatureFunction = InFunction;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Enum(UEnum* InEnum, FProperty* InUnderlying, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FEnumProperty>(InName, InOuter);
	FEnumProperty* EnumProp = Ret.As<FEnumProperty>();

	EnumProp->SetEnum(InEnum);
	InUnderlying->Owner = EnumProp;
	EnumProp->AddCppProperty(InUnderlying);

	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Enum(UEnum* InEnum, const FName InName, FFieldVariant InOuter)
{
	return Enum(InEnum, new FByteProperty(InEnum, TEXT("UnderlyingType"), RF_NoFlags), InName, InOuter);
}

FDcPropertyBuilder FDcPropertyBuilder::Byte(UEnum* InEnum, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FByteProperty>(InName, InOuter);
	Ret.As<FByteProperty>()->Enum = InEnum;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Bool(uint32 InSize, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FBoolProperty>(InName, InOuter);
	Ret.As<FBoolProperty>()->SetBoolSize(InSize, true);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Array(FProperty* InInner, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FArrayProperty>(InName, InOuter);
	Ret.As<FArrayProperty>()->Inner = InInner;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Set(FProperty* InInner, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FSetProperty>(InName, InOuter);
	Ret.As<FSetProperty>()->ElementProp = InInner;
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::Map(FProperty* InKey, FProperty* InValue, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FMapProperty>(InName, InOuter);
	Ret.As<FMapProperty>()->KeyProp = InKey;
	Ret.As<FMapProperty>()->ValueProp = InValue;
	return Ret;
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
FDcPropertyBuilder FDcPropertyBuilder::ObjectPtr(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FObjectPtrProperty>(InName, InOuter);
	Ret.As<FObjectPtrProperty>()->SetPropertyClass(InClass);
	return Ret;
}

FDcPropertyBuilder FDcPropertyBuilder::ClassPtr(UClass* InClass, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FClassPtrProperty>(InName, InOuter);
	Ret.As<FClassPtrProperty>()->SetPropertyClass(UClass::StaticClass());
	Ret.As<FClassPtrProperty>()->SetMetaClass(InClass);
	return Ret;
}
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
FDcPropertyBuilder FDcPropertyBuilder::Optional(FProperty* InInner, const FName InName, FFieldVariant InOuter)
{
	FDcPropertyBuilder Ret = Make<FOptionalProperty>(InName, InOuter);
	Ret.As<FOptionalProperty>()->SetValueProperty(InInner);
	return Ret;
}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

}	// namespace DcPropertyUtils
