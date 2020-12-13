#include "DataConfig/Property/DcPropertyStatesCommon.h"
#include "DataConfig/Property/DcPropertyUtils.h"

void DcPropertyHighlight::FormatNil(TArray<FString>& OutSegments, EFormatSeg SegType)
{
	OutSegments.Add(TEXT("<nil>"));
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

void DcPropertyHighlight::FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, FMapProperty* MapProperty, uint16 Index, bool bIsKeyOrValue)
{
	check(MapProperty);
	FString Seg = FString::Printf(TEXT("(%s)%s"),
		*DcPropertyUtils::GetFormatPropertyTypeName(MapProperty),
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
		*DcPropertyUtils::GetFormatPropertyTypeName(ArrayProperty),
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
		*DcPropertyUtils::GetFormatPropertyTypeName(SetProperty),
		*SetProperty->ElementProp->GetName()
	);

	if (bIsItem)
		Seg.Append(FString::Printf(TEXT("[%d]"), Index));

	OutSegments.Add(MoveTemp(Seg));
}
