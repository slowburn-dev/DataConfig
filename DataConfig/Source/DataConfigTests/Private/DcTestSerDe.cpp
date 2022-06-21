#include "DcTestSerDe.h"
#include "UObject/Package.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

void FDcTestStructEnumFlag1::MakeFixture()
{
#if !WITH_METADATA
	UEnum* EnumClass = StaticEnum<EDcTestEnumFlag>();
	check(EnumClass);
	DcAutomationUtils::AmendMetaData(EnumClass, TEXT("Bitflags"), TEXT(""));
#endif

	EnumFlagField1 = EDcTestEnumFlag::None;
	EnumFlagField2 = EDcTestEnumFlag::One | EDcTestEnumFlag::Three | EDcTestEnumFlag::Five;
}

void FDcTestStructShapeContainer1::MakeFixtureEmpty()
{
	ShapeField1 = NewObject<UDcShapeBox>();;
	ShapeField2 = NewObject<UDcShapeSquare>();
	ShapeField3 = nullptr;
}

void FDcTestStructShapeContainer1::MakeFixture()
{
	MakeFixtureEmpty();

	UDcShapeBox* Shape1 = CastChecked<UDcShapeBox>(ShapeField1);
	Shape1->ShapeName = TEXT("Box1");
	Shape1->Height = 17.5;
	Shape1->Width = 1.9375;

	UDcShapeSquare* Shape2 = CastChecked<UDcShapeSquare>(ShapeField2);
	Shape2->ShapeName = TEXT("Square1");
	Shape2->Radius = 1.75;
}

void FDcTestStructRefs1::MakeFixture()
{
	UObject* TestsObject = StaticFindObject(UObject::StaticClass(), nullptr, TEXT("/Script/DataConfigTests"));

	ObjectField1 = TestsObject;
	ObjectField2 = nullptr;
	SoftField1 = TestsObject;
	SoftField2 = nullptr;
	WeakField1 = TestsObject;
	WeakField2 = nullptr;
	LazyField1 = TestsObject;
	LazyField2 = nullptr;
}

void FDcTestStructRefs2::MakeFixture()
{
	UClass* DynamicMetaClass = FindObject<UClass>(ANY_PACKAGE, TEXT("DynamicClass"));

	RawClassField1 = DynamicMetaClass;
	RawClassField2 = nullptr;
	SubClassField1 = DynamicMetaClass;
	SubClassField2 = nullptr;
	SoftClassField1 = DynamicMetaClass;
	SoftClassField2 = nullptr;
}

void FDcTestStructArrayDim1::MakeFixture()
{
	NameArr[0] = TEXT("Name0");
	NameArr[1] = TEXT("Name1");

	StringArr[0] = TEXT("Str0");
	StringArr[1] = TEXT("Str1");

	FloatArr[0] = 12.3f;
	FloatArr[1] = 23.4f;

	Int8Arr[0] = 23;
	Int8Arr[1] = -34;

	EnumFlagArr[0] = EDcTestEnumFlag::None;
	EnumFlagArr[1] = EDcTestEnumFlag::One | EDcTestEnumFlag::Three | EDcTestEnumFlag::Five;

	StructArr[0] = { {TEXT("One"), TEXT("Two")}, {1, 2} };
	StructArr[1] = { {TEXT("Three"), TEXT("Four")}, {3, 4} };

}

void FDcTestStructMaps::MakeFixture()
{
#if !WITH_METADATA
	UEnum* EnumClass = StaticEnum<EDcTestEnumFlag>();
	check(EnumClass);
	DcAutomationUtils::AmendMetaData(EnumClass, TEXT("Bitflags"), TEXT(""));
#endif

	ColorKeyMap.Add(FColor::Red, TEXT("Red"));
	ColorKeyMap.Add(FColor::Green, TEXT("Green"));
	ColorKeyMap.Add(FColor::Blue, TEXT("Blue"));

	EnumFlagsMap.Add(EDcTestEnumFlag::None, TEXT("None"));
	EnumFlagsMap.Add(EDcTestEnumFlag::One | EDcTestEnumFlag::Three, TEXT("One | Three"));
	EnumFlagsMap.Add(EDcTestEnumFlag::Five, TEXT("Five"));
}
