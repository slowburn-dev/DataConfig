#include "DataConfig/Property/DcPropertyStatesCommon.h"
#include "DataConfig/Property/DcPropertyUtils.h"

FName DC_TRANSIENT_ARRAY = FName(TEXT("DcTransientArray"));
FName DC_TRANSIENT_SET = FName(TEXT("DcTransientSet"));
FName DC_TRANSIENT_MAP = FName(TEXT("DcTransientMap"));

void DcPropertyHighlight::FormatNone(TArray<FString>& OutSegments, EFormatSeg SegType)
{
	OutSegments.Add(TEXT("<none>"));
}

void DcPropertyHighlight::FormatScalar(TArray<FString>& OutSegments, EFormatSeg SegType, FProperty* Property, int Index, bool bIsItem)
{
	FString Seg = FString::Printf(TEXT("(%s%s)%s"),
		*DcPropertyUtils::GetFormatPropertyTypeName(Property),
		Property->ArrayDim > 1 ? *FString::Printf(TEXT("[%d]"), Property->ArrayDim) : TEXT(""),
		*Property->GetName());

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ObjectName, UClass* Class, FProperty* Property)
{
	if (SegType != EFormatSeg::ParentIsContainer)
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*DcPropertyUtils::GetFormatPropertyTypeName(Class),
			*DcPropertyUtils::SafeNameToString(ObjectName)
		));
	}

	if (Property != nullptr
		&& (SegType == EFormatSeg::Last || DcPropertyUtils::IsScalarProperty(Property)))
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*DcPropertyUtils::GetFormatPropertyTypeName(Property),
			*Property->GetName()));
	}
}

void DcPropertyHighlight::FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& StructName, UScriptStruct* StructClass, FProperty* Property)
{
	if (SegType != EFormatSeg::ParentIsContainer)
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*DcPropertyUtils::GetFormatPropertyTypeName(StructClass),
			*DcPropertyUtils::SafeNameToString(StructName)
		));
	}

	if (Property != nullptr
		&& (SegType == EFormatSeg::Last || DcPropertyUtils::IsScalarProperty(Property)))
	{
		OutSegments.Add(FString::Printf(TEXT("(%s)%s"),
			*DcPropertyUtils::GetFormatPropertyTypeName(Property),
			*Property->GetName()));
	}
}

void DcPropertyHighlight::FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& MapName, FProperty* KeyProperty, FProperty* ValueProperty, int Index, bool bIsKeyOrValue)
{
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*DcPropertyUtils::FormatMapTypeName(KeyProperty, ValueProperty),
		*MapName.ToString()
	);

	if (bIsKeyOrValue)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ArrayName, FProperty* InnerProperty, int Index, bool bIsItem)
{
	check(InnerProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*DcPropertyUtils::FormatArrayTypeName(InnerProperty),
		*ArrayName.ToString()
	);

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatSet(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& SetName, FProperty* ElementProperty, int Index, bool bIsItem)
{
	check(ElementProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*DcPropertyUtils::FormatSetTypeName(ElementProperty),
		*SetName.ToString()
	);

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}

void DcPropertyHighlight::FormatOptional(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& OptionalName, FProperty* ValueProperty)
{
	check(ValueProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*DcPropertyUtils::FormatOptionalTypeName(ValueProperty),
		*OptionalName.ToString()
	);

	OutSegments.Add(MoveTemp(Seg));
}
