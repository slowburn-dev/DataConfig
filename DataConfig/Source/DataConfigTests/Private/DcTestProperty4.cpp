#include "DcTestProperty4.h"
#include "DcTestProperty.h"
#include "DcTestProperty2.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"

static UDcShapeBox* _MakeBox()
{
	UDcShapeBox* Box = NewObject<UDcShapeBox>();
	Box->ShapeName = TEXT("MyBox");
	Box->Height = 12.3f;
	Box->Width = 23.4f;
	return Box;
}

static UDcShapeSquare* _MakeSquare()
{
	UDcShapeSquare* Square = NewObject<UDcShapeSquare>();
	Square->ShapeName = TEXT("MySquare");
	Square->Radius = 34.5f;
	return Square;
}

void FDcTestObjectRefs1::MakeFixture()
{
	UDcTestClass1* Obj1 = NewObject<UDcTestClass1>();
	Obj1->IntField = 123;
	Obj1->StrField = TEXT("Foo");

	UDcTestClass1* Obj2 = NewObject<UDcTestClass1>();
	Obj1->IntField = 234;
	Obj1->StrField = TEXT("Bar");

	UDcTestClass1* Obj3 = NewObject<UDcTestClass1>();
	Obj1->IntField = 345;
	Obj1->StrField = TEXT("Dar");

	RefObjectArr.Add(Obj1);
	RefObjectArr.Add(nullptr);
	RefObjectArr.Add(Obj2);
	RefObjectArr.Add(Obj3);

	RefObjectSet.Add(Obj1);
	RefObjectSet.Add(nullptr);
	RefObjectSet.Add(Obj2);
	RefObjectSet.Add(Obj3);

	RefObjectMap.Add(TEXT("One"), Obj1);
	RefObjectMap.Add(TEXT("Two"), Obj2);
	RefObjectMap.Add(TEXT("Three"), Obj3);
	RefObjectMap.Add(TEXT("Null"), nullptr);

	InlineObjectArr.Add(_MakeBox());
	InlineObjectArr.Add(_MakeSquare());
	InlineObjectArr.Add(nullptr);

	InlineObjectSet.Add(_MakeBox());
	InlineObjectSet.Add(_MakeSquare());
	InlineObjectSet.Add(nullptr);

	InlineObjectMap.Add(TEXT("Box"), _MakeBox());
	InlineObjectMap.Add(TEXT("Square"), _MakeSquare());
	InlineObjectMap.Add(TEXT("Null"), nullptr);
}

void FDcTestArrayDim1::MakeFixture()
{
	NameArr[0] = FName(TEXT("Foo"));
	NameArr[1] = FName(TEXT("Bar"));

	StringArr[0] = TEXT("Foo");
	StringArr[1] = TEXT("Bar");
	StringArr[2] = TEXT("Baz");

	TextArr[0] = FText::FromString(TEXT("Foo"));
	TextArr[1] = FText::FromString(TEXT("Bar"));
	TextArr[2] = FText::FromString(TEXT("Baz"));
	TextArr[3] = FText::FromString(TEXT("Dar"));

	EnumArr[0] = EDcTestEnum1::Bar;
	EnumArr[1] = EDcTestEnum1::Tard;

	FloatArr[0] = 1.23f;
	FloatArr[1] = 2.34f;

	DoubleArr[0] = -1.23;
	DoubleArr[1] = -2.34;
	DoubleArr[2] = -3.45;

	Int8Arr[0] = -10;
	Int8Arr[1] = -11;

	Int16Arr[0] = -10;
	Int16Arr[1] = -11;
	Int16Arr[2] = -12;

	Int32Arr[0] = -10;
	Int32Arr[1] = -11;
	Int32Arr[2] = -12;
	Int32Arr[3] = -13;

	Int64Arr[0] = -10;
	Int64Arr[1] = -11;
	Int64Arr[2] = -12;
	Int64Arr[3] = -13;
	Int64Arr[4] = -14;

	UInt8Arr[0] = 10;
	UInt8Arr[1] = 11;

	UInt16Arr[0] = 10;
	UInt16Arr[1] = 11;
	UInt16Arr[2] = 12;

	UInt32Arr[0] = 10;
	UInt32Arr[1] = 11;
	UInt32Arr[2] = 12;
	UInt32Arr[3] = 13;

	UInt64Arr[0] = 10;
	UInt64Arr[1] = 11;
	UInt64Arr[2] = 12;
	UInt64Arr[3] = 13;
	UInt64Arr[4] = 14;

	UDcTestClass1* Obj1 = NewObject<UDcTestClass1>();
	Obj1->IntField = 123;
	Obj1->StrField = TEXT("Foo");

	ObjectRefArr[0] = Obj1;
	ObjectRefArr[1] = nullptr;

	ObjectInlineArr[0] = _MakeBox();
	ObjectInlineArr[1] = _MakeSquare();
	ObjectInlineArr[2] = nullptr;

	ClassArr[0] = UDcTestClass1::StaticClass();
	ClassArr[1] = UDcShapeSquare::StaticClass();
	ClassArr[2] = nullptr;

	WeakObjectArr[0] = ObjectRefArr[0];
	WeakObjectArr[1] = ObjectRefArr[1];

	LazyObjectArr[0] = ObjectRefArr[0];
	LazyObjectArr[1] = ObjectRefArr[1];
	LazyObjectArr[2] = nullptr;

	SoftObjectArr[0] = ObjectRefArr[0];
	SoftObjectArr[1] = ObjectRefArr[1];
	SoftObjectArr[2] = nullptr;
	SoftObjectArr[3] = ObjectRefArr[0];

	SoftClassArr[0] = UDcTestClass1::StaticClass();
	SoftClassArr[1] = UDcShapeSquare::StaticClass();

	UObject* InterfaceObj = NewObject<UDcTestInterface1Beta>();
	InterfaceArr[0] = InterfaceObj;
	InterfaceArr[1] = nullptr;
	InterfaceArr[2] = InterfaceObj;

	FieldPathArr[0] = DcPropertyUtils::FindEffectivePropertyByName(FDcTestStruct1::StaticStruct(), TEXT("NameField"));
	FieldPathArr[1] = DcPropertyUtils::FindEffectivePropertyByName(FDcTestStruct1::StaticStruct(), TEXT("BoolField"));
	FieldPathArr[2] = DcPropertyUtils::FindEffectivePropertyByName(FDcTestStruct1::StaticStruct(), TEXT("StringField"));
	FieldPathArr[3] = {};

	UDcTestDelegateClass1* DelegateObj = NewObject<UDcTestDelegateClass1>();
	DelegateArr[0].BindDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnOne);
	DelegateArr[1].Clear();

	DynMulticastArr[0].AddDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnNone);
	DynMulticastArr[1].Clear();
	DynMulticastArr[2].AddDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnNone);

	StructArr[0].NameField = TEXT("Name0");
	StructArr[0].StrField = TEXT("Str0");

	StructArr[1].NameField = TEXT("Name0");
	StructArr[1].StrField = TEXT("Str0");
}

void UDcTestArrayDim2::MakeFixture()
{
	NameArr[0] = TEXT("Foo");
	NameArr[0] = TEXT("Bar");

	StringArr[1] = TEXT("These are my");
	StringArr[2] = TEXT("Twisted words");

	InlineObjectArr[0] = NewObject<UDcTestArrayDimInner2>(this);
	InlineObjectArr[0]->InnerNameArr[0] = TEXT("InnerFoo"); 
	InlineObjectArr[0]->InnerNameArr[1] = TEXT("InnerBar"); 

	InlineObjectArr[0]->InnerIntArr[0] = 123;
	InlineObjectArr[0]->InnerIntArr[1] = 234;

	InlineObjectArr[1] = nullptr;
}

void FDcTestHighlight::MakeFixture()
{
	NameField = "Foo";
	NameArr[0] = "Bar";
	NameArr[1] = "Dar";

	StrArr.Add(TEXT("Arr1"));
	StrArr.Add(TEXT("Arr2"));

	StrMap.Add(TEXT("One"), TEXT("1"));
	StrMap.Add(TEXT("Two"), TEXT("2"));

	StrSet.Add(TEXT("A"));
	StrSet.Add(TEXT("B"));
}

DC_TEST("DataConfig.Core.Property.ContainerRoots")
{
	FString SrcStr = TEXT("These are my twisted words");

	uint64 SrcU64 = TNumericLimits<uint64>::Max();

	TArray<FDcTestStructSimple> SrcArr = {
		{TEXT("Foo"), TEXT("Bar")},
		{TEXT("Doo"), TEXT("Dar")},
		{TEXT("Goo"), TEXT("Gar")},
	};

	TSet<EDcTestEnum1> SrcSet = {
		EDcTestEnum1::Foo,
		EDcTestEnum1::Bar,
		EDcTestEnum1::Tard,
	};

	TMap<FString, int> SrcMap = {
		{TEXT("One"), 1},
		{TEXT("Two"), 2},
		{TEXT("Three"), 3},
	};

	using namespace DcPropertyUtils;

	auto StrProp = FDcPropertyBuilder::Str().LinkOnScope();		
	auto U64Prop = FDcPropertyBuilder::UInt64().LinkOnScope();

	auto ArrInnerProp = FDcPropertyBuilder::Struct(FDcTestStructSimple::StaticStruct()).LinkOnScope();
	auto SetInnerProp = FDcPropertyBuilder::Enum(
			StaticEnum<EDcTestEnum1>(),
			FDcPropertyBuilder::Int64()
		).LinkOnScope();	

	auto MapKeyProp = FDcPropertyBuilder::Str().LinkOnScope();
	auto MapValueProp =  FDcPropertyBuilder::Int().LinkOnScope();

	FString DestStr;
	uint64 DestU64;
	TArray<FDcTestStructSimple> DestArr;
	TSet<EDcTestEnum1> DestSet;
	TMap<FString, int> DestMap;

	UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(StrProp.Get(), &SrcStr),
		FDcPropertyDatum(StrProp.Get(), &DestStr)
	));

	UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(U64Prop.Get(), &SrcU64),
		FDcPropertyDatum(U64Prop.Get(), &DestU64)
	));

	{
		FDcPropertyReader Reader(FDcPropertyReader::Array, ArrInnerProp.Get(), &SrcArr);
		FDcPropertyWriter Writer(FDcPropertyWriter::Array, ArrInnerProp.Get(), &DestArr);
		UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisit(Reader, Writer));

		auto ArrProp = FDcPropertyBuilder::Array(
				FDcPropertyBuilder::Struct(FDcTestStructSimple::StaticStruct())
			).LinkOnScope();

		UTEST_OK("ContainerRoot Roundtrip", DcAutomationUtils::TestReadDatumEqual(
			FDcPropertyDatum(ArrProp.Get(), &SrcArr),
			FDcPropertyDatum(ArrProp.Get(), &DestArr)
		));
	}

	{
		FDcPropertyReader Reader(FDcPropertyReader::Set, SetInnerProp.Get(), &SrcSet);
		FDcPropertyWriter Writer(FDcPropertyWriter::Set, SetInnerProp.Get(), &DestSet);
		UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisit(Reader, Writer));

		auto SetProp = FDcPropertyBuilder::Set(
				FDcPropertyBuilder::Enum(
					StaticEnum<EDcTestEnum1>(),
					FDcPropertyBuilder::Int64()
					)
				).LinkOnScope();

		UTEST_OK("ContainerRoot Roundtrip", DcAutomationUtils::TestReadDatumEqual(
			FDcPropertyDatum(SetProp.Get(), &SrcSet),
			FDcPropertyDatum(SetProp.Get(), &DestSet)
		));
	}

	{
		FDcPropertyReader Reader(MapKeyProp.Get(), MapValueProp.Get(), &SrcMap);
		FDcPropertyWriter Writer(MapKeyProp.Get(), MapValueProp.Get(), &DestMap);
		UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisit(Reader, Writer));

		auto MapProp = FDcPropertyBuilder::Map(
				FDcPropertyBuilder::Str(),
				FDcPropertyBuilder::Int()
			).LinkOnScope();

		UTEST_OK("ContainerRoot Roundtrip", DcAutomationUtils::TestReadDatumEqual(
			FDcPropertyDatum(MapProp.Get(), &SrcMap),
			FDcPropertyDatum(MapProp.Get(), &DestMap)
		));
	}

	return true;
}

DC_TEST("DataConfig.Core.Property.StackScalarRoots")
{
	int SrcIntVal = 253;
	int SrcIntArr[5] = {1,2,3,4,5};

	using namespace DcPropertyUtils;

	auto IntProp = FDcPropertyBuilder::Int().LinkOnScope();		
	auto IntArrProp = FDcPropertyBuilder::Int().ArrayDim(5).LinkOnScope();

	int DestIntVal;
	int DestIntArr[5];

	UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(IntProp.Get(), &SrcIntVal),
		FDcPropertyDatum(IntProp.Get(), &DestIntVal)
	));

	UTEST_OK("ContainerRoot Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(IntArrProp.Get(), &SrcIntArr),
		FDcPropertyDatum(IntArrProp.Get(), &DestIntArr)
	));

	return true;
}


FDcResult _PeekVisitCreateInlineObjects(FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl)
{
	if (Next == EDcDataEntry::ClassRoot)
	{
		FDcPropertyWriter* PropWriter = Visitor->Writer->CastByIdChecked<FDcPropertyWriter>();

		FFieldVariant WriterProp;
		DC_TRY(PropWriter->PeekWriteProperty(&WriterProp));

		FObjectProperty* ObjProp = nullptr;
		if (WriterProp.IsA<FObjectProperty>())
			ObjProp = CastFieldChecked<FObjectProperty>(WriterProp.ToFieldUnsafe());

		if (ObjProp && DcPropertyUtils::IsSubObjectProperty(ObjProp))
		{
			FDcPropertyReader* PropReader = Visitor->Reader->CastByIdChecked<FDcPropertyReader>();

			FFieldVariant SrcProp;
			void* SrcPtr;
			DC_TRY(PropReader->PeekReadProperty(&SrcProp));
			DC_TRY(PropReader->PeekReadDataPtr(&SrcPtr));

			UObject* InlineObj = CastFieldChecked<FObjectProperty>(SrcProp.ToField())->GetPropertyValue(SrcPtr);
			if (InlineObj != nullptr)
			{
				//	construct inline object when src is non null

				//	consume root to match with writer
				DC_TRY(PropReader->ReadClassRoot());

				//	manually write in inline object
				check(InlineObj->GetClass()->IsChildOf(ObjProp->PropertyClass));
				UObject* SubObject = NewObject<UObject>(GetTransientPackage(), InlineObj->GetClass());

				FDcPropertyDatum Datum;
				DC_TRY(PropWriter->WriteDataEntry(FObjectProperty::StaticClass(), Datum));

				ObjProp->SetPropertyValue(Datum.DataPtr, SubObject);
				DC_TRY(PropWriter->PushTopClassPropertyState(Datum));

				FDcClassAccess WriteClassStat{ FDcClassAccess::EControl::ExpandObject };
				DC_TRY(PropWriter->WriteClassRootAccess(WriteClassStat));

				OutControl = EPipeVisitControl::SkipContinue;
			}
		}
	}

	return DcOk();
}

DC_TEST("DataConfig.Core.Property.ContainerObjectRefs")
{
	FDcTestObjectRefs1 Src;
	Src.MakeFixture();

	FDcTestObjectRefs1 Dest;

	FDcPropertyReader Reader{FDcPropertyDatum(&Src)};
	FDcPropertyWriter Writer{FDcPropertyDatum(&Dest)};

	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	RoundtripVisit.PeekVisit.BindStatic(_PeekVisitCreateInlineObjects);

	UTEST_OK("ContainerObjectRefs Roundtrip", RoundtripVisit.PipeVisit());

	return true;
}


DC_TEST("DataConfig.Core.Property.ScalarArray")
{
	FDcTestArrayDim1 Src;
	Src.MakeFixture();

	FDcTestArrayDim1 Dest;

	FDcPropertyReader Reader{FDcPropertyDatum(&Src)};
	FDcPropertyWriter Writer{FDcPropertyDatum(&Dest)};

	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	RoundtripVisit.PeekVisit.BindStatic(_PeekVisitCreateInlineObjects);

	UTEST_OK("ScalarArray Roundtrip", RoundtripVisit.PipeVisit());

	return true;
}

DC_TEST("DataConfig.Core.Property.ScalarArray2")
{
	UDcTestArrayDim2* Src = NewObject<UDcTestArrayDim2>();
	Src->MakeFixture();

	UDcTestArrayDim2* Dest = NewObject<UDcTestArrayDim2>();

	FDcPropertyReader Reader{FDcPropertyDatum(Src)};
	FDcPropertyWriter Writer{FDcPropertyDatum(Dest)};

	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	RoundtripVisit.PeekVisit.BindStatic(_PeekVisitCreateInlineObjects);

	UTEST_OK("ScalarArray Roundtrip", RoundtripVisit.PipeVisit());

	return true;
}

DC_TEST("DataConfig.Core.Property.HighlightFormat")
{
	FDcTestHighlight Val;
	Val.MakeFixture();

	FDcPropertyReader Reader{FDcPropertyDatum(&Val)};
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root"));

	UTEST_OK("Highlight Format", Reader.ReadStructRoot());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(FName)NameField"));

	UTEST_OK("Highlight Format", Reader.ReadName(nullptr));
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(FName)NameField"));

	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(FName)NameArr"));

	UTEST_OK("Highlight Format", Reader.ReadName(nullptr));
	UTEST_OK("Highlight Format", Reader.ReadArrayRoot());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(FName[2])NameArr[0]"));

	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(FName[2])NameArr[1]"));

	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.ReadArrayEnd());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TArray<FString>)StrArr"));

	UTEST_OK("Highlight Format", Reader.ReadName(nullptr));
	UTEST_OK("Highlight Format", Reader.ReadArrayRoot());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TArray<FString>)StrArr[0]"));

	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TArray<FString>)StrArr[1]"));
	
	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.ReadArrayEnd());

	UTEST_OK("Highlight Format", Reader.ReadName(nullptr));
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TMap<FString, FString>)StrMap"));

	UTEST_OK("Highlight Format", Reader.ReadMapRoot());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TMap<FString, FString>)StrMap[0]"));

	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.ReadMapEnd());

	UTEST_OK("Highlight Format", Reader.ReadName(nullptr));
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TSet<FString>)StrSet"));

	UTEST_OK("Highlight Format", Reader.ReadSetRoot());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: (FDcTestHighlight)$root.(TSet<FString>)StrSet[0]"));

	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.SkipRead());
	UTEST_OK("Highlight Format", Reader.ReadSetEnd());
	UTEST_OK("Highlight Format", Reader.ReadStructEnd());
	UTEST_EQUAL("Highlight Format", Reader.FormatHighlight().Formatted,
		TEXT("Reading property: <nil>"));

	return true;
}

DC_TEST("DataConfig.Core.Property.Blob2")
{
	FDcTestScalarArrayBlob Src; 
	Src.ColorField = FColor::Red;
	Src.ColorArr[0] = FColor::Green;
	Src.ColorArr[1] = FColor::Blue;

	FDcTestScalarArrayBlob Dest;

	FDcPropertyDatum SourceDatum(&Src);
	FDcPropertyDatum DestDatum(&Dest);

	FDcPropertyReader Reader(SourceDatum);
	FDcPropertyWriter Writer(DestDatum);

	{
		//	manual roundtrip
		UTEST_OK("Property Blob2", Reader.ReadStructRoot());
		UTEST_OK("Property Blob2", Writer.WriteStructRoot());

		UTEST_OK("Property Blob2", Reader.ReadName(nullptr));
		UTEST_OK("Property Blob2", Writer.WriteName(TEXT("ColorField")));

		{
			FDcBlobViewData BlobView;
			bool bCanCoerce;

			UTEST_OK("Property Blob2", Reader.Coercion(EDcDataEntry::Blob, &bCanCoerce));
			UTEST_TRUE("Property Blob2", bCanCoerce);
			UTEST_OK("Property Blob2", Reader.ReadBlob(&BlobView));

			bool bWriteOk;
			UTEST_OK("Property Blob2", Writer.PeekWrite(EDcDataEntry::Blob, &bWriteOk));
			UTEST_TRUE("Property Blob2", bWriteOk);
			UTEST_OK("Property Blob2", Writer.WriteBlob(BlobView));
		}

		UTEST_OK("Property Blob2", Reader.ReadName(nullptr));
		UTEST_OK("Property Blob2", Writer.WriteName(TEXT("ColorArr")));

		{
			FDcBlobViewData BlobView;
			bool bCanCoerce;

			UTEST_OK("Property Blob2", Reader.Coercion(EDcDataEntry::Blob, &bCanCoerce));
			UTEST_TRUE("Property Blob2", bCanCoerce);
			UTEST_OK("Property Blob2", Reader.ReadBlob(&BlobView));

			bool bWriteOk;
			UTEST_OK("Property Blob2", Writer.PeekWrite(EDcDataEntry::Blob, &bWriteOk));
			UTEST_TRUE("Property Blob2", bWriteOk);
			UTEST_OK("Property Blob2", Writer.WriteBlob(BlobView));
		}

		UTEST_OK("Property Blob2", Reader.ReadStructEnd());
		UTEST_OK("Property Blob2", Writer.WriteStructEnd());
	}

	UTEST_OK("Property Blob2", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}

DC_TEST("DataConfig.Core.Property.PropertyBuilder")
{
	FString SrcStr = TEXT("These are my twisted words");

	uint64 SrcU64 = TNumericLimits<uint64>::Max();

	TArray<FDcTestStructSimple> SrcArr = {
		{TEXT("Foo"), TEXT("Bar")},
		{TEXT("Doo"), TEXT("Dar")},
		{TEXT("Goo"), TEXT("Gar")},
	};

	TSet<EDcTestEnum1> SrcSet = {
		EDcTestEnum1::Foo,
		EDcTestEnum1::Bar,
		EDcTestEnum1::Tard,
	};

	TMap<FString, int> SrcMap = {
		{TEXT("One"), 1},
		{TEXT("Two"), 2},
		{TEXT("Three"), 3},
	};

	using namespace DcPropertyUtils;

	auto StrProp = FDcPropertyBuilder::Str().LinkOnScope();		
	auto U64Prop = FDcPropertyBuilder::UInt64().LinkOnScope();

	auto ArrProp = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::Struct(FDcTestStructSimple::StaticStruct())
		).LinkOnScope();

	auto SetProp = FDcPropertyBuilder::Set(
			FDcPropertyBuilder::Enum(
				StaticEnum<EDcTestEnum1>(),
				FDcPropertyBuilder::Int64()
				)
			).LinkOnScope();

	auto MapProp = FDcPropertyBuilder::Map(
			FDcPropertyBuilder::Str(),
			FDcPropertyBuilder::Int()
		).LinkOnScope();

	FString DestStr;
	uint64 DestU64;
	TArray<FDcTestStructSimple> DestArr;
	TSet<EDcTestEnum1> DestSet;
	TMap<FString, int> DestMap;

	UTEST_OK("Builder Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(StrProp.Get(), &SrcStr),
		FDcPropertyDatum(StrProp.Get(), &DestStr)
	));

	UTEST_OK("Builder Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(U64Prop.Get(), &SrcU64),
		FDcPropertyDatum(U64Prop.Get(), &DestU64)
	));

	UTEST_OK("Builder Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(ArrProp.Get(), &SrcArr),
		FDcPropertyDatum(ArrProp.Get(), &DestArr)
	));

	UTEST_OK("Builder Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(SetProp.Get(), &SrcSet),
		FDcPropertyDatum(SetProp.Get(), &DestSet)
	));

	UTEST_OK("Builder Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(MapProp.Get(), &SrcMap),
		FDcPropertyDatum(MapProp.Get(), &DestMap)
	));

	return true;
}

DC_TEST("DataConfig.Core.Property.PropertyBuilder2")
{
	using namespace DcPropertyUtils;

	FDcTestStruct1 Src;
	Src.MakeFixture();

	FDcTestStruct1 Dest;

	{
		auto BoolProp = FDcPropertyBuilder::Bool().LinkOnScope();
		FDcJsonReader Reader(TEXT("true"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(BoolProp.Get(), &Dest.BoolField)));
	}

	{
		auto NameProp = FDcPropertyBuilder::Name().LinkOnScope();
		FDcJsonReader Reader(TEXT("\"AName\""));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(NameProp.Get(), &Dest.NameField)));
	}

	{
		auto StrProp = FDcPropertyBuilder::Str().LinkOnScope();
		FDcJsonReader Reader(TEXT("\"AStr\""));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(StrProp.Get(), &Dest.StringField)));
	}

	{
		auto TextProp = FDcPropertyBuilder::Text().LinkOnScope();
		FDcJsonReader Reader(TEXT("\"AText\""));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(TextProp.Get(), &Dest.TextField)));
	}

	{
		auto EnumProp = FDcPropertyBuilder::Enum(
			StaticEnum<EDcTestEnum1>(), 
			FDcPropertyBuilder::Int64()
			).LinkOnScope();
		FDcJsonReader Reader(TEXT("\"Tard\""));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(EnumProp.Get(), &Dest.EnumField)));
	}

	{
		auto FloatProp = FDcPropertyBuilder::Float().LinkOnScope();
		FDcJsonReader Reader(TEXT("17.5"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(FloatProp.Get(), &Dest.FloatField)));
	}

	{
		auto DoubleProp = FDcPropertyBuilder::Double().LinkOnScope();
		FDcJsonReader Reader(TEXT("19.375"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(DoubleProp.Get(), &Dest.DoubleField)));
	}

	{
		auto Int8Prop = FDcPropertyBuilder::Int8().LinkOnScope();
		FDcJsonReader Reader(TEXT("-43"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(Int8Prop.Get(), &Dest.Int8Field)));
	}

	{
		auto Int16Prop = FDcPropertyBuilder::Int16().LinkOnScope();
		FDcJsonReader Reader(TEXT("-2243"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(Int16Prop.Get(), &Dest.Int16Field)));
	}

	{
		auto Int32Prop = FDcPropertyBuilder::Int().LinkOnScope();
		FDcJsonReader Reader(TEXT("-23415"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(Int32Prop.Get(), &Dest.Int32Field)));
	}

	{
		auto Int64Prop = FDcPropertyBuilder::Int64().LinkOnScope();
		FDcJsonReader Reader(TEXT("-1524523"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(Int64Prop.Get(), &Dest.Int64Field)));
	}

	{
		auto UInt8Prop = FDcPropertyBuilder::Byte().LinkOnScope();
		FDcJsonReader Reader(TEXT("213"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(UInt8Prop.Get(), &Dest.UInt8Field)));
	}

	{
		auto UInt16Prop = FDcPropertyBuilder::UInt16().LinkOnScope();
		FDcJsonReader Reader(TEXT("2243"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(UInt16Prop.Get(), &Dest.UInt16Field)));
	}

	{
		auto UInt32Prop = FDcPropertyBuilder::UInt32().LinkOnScope();
		FDcJsonReader Reader(TEXT("23415"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(UInt32Prop.Get(), &Dest.UInt32Field)));
	}

	{
		auto UInt64Prop = FDcPropertyBuilder::UInt64().LinkOnScope();
		FDcJsonReader Reader(TEXT("1524523"));
		UTEST_OK("Builder Roundtrip2", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(UInt64Prop.Get(), &Dest.UInt64Field)));
	}

	UTEST_OK("Builder Roundtrip2", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Src), FDcPropertyDatum(&Dest)));

	return true;
}

DC_TEST("DataConfig.Core.Property.PropertyBuilder3")
{
	using namespace DcPropertyUtils;	

	{
		UObject* Src = NewObject<UDcTestClass1>();
		UObject* Dest;

		auto ObjProp = FDcPropertyBuilder::Object(UDcTestClass1::StaticClass()).LinkOnScope();
		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(ObjProp.Get(), &Src),
			FDcPropertyDatum(ObjProp.Get(), &Dest)
		));
	}

	{
		UClass* Src = UDcTestClass1::StaticClass();
		UClass* Dest;

		auto ClassProp = FDcPropertyBuilder::Class(UDcTestClass1::StaticClass()).LinkOnScope();
		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(ClassProp.Get(), &Src),
			FDcPropertyDatum(ClassProp.Get(), &Dest)
		));
	}

	{
		FDcTestStructSimple Src {
			TEXT("Foo"), TEXT("Bar")
		};
		FDcTestStructSimple Dest;

		auto StructProp = FDcPropertyBuilder::Struct(FDcTestStructSimple::StaticStruct()).LinkOnScope();
		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(StructProp.Get(), &Src),
			FDcPropertyDatum(StructProp.Get(), &Dest)
		));
	}

	{
		TSoftObjectPtr<UPackage> Src = FindObject<UPackage>(ANY_PACKAGE, TEXT("/Script/DataConfigTests"), true);
		TSoftObjectPtr<UPackage> Dest;

		auto SoftObjectProp = FDcPropertyBuilder::SoftObject(UPackage::StaticClass()).LinkOnScope();
		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(SoftObjectProp.Get(), &Src),
			FDcPropertyDatum(SoftObjectProp.Get(), &Dest)
		));
	}

	{
		TSoftClassPtr<UDcBaseShape> Src = UDcShapeBox::StaticClass();
		TSoftClassPtr<UDcBaseShape> Dest;

		auto SoftClassProp = FDcPropertyBuilder::SoftObject(UDcShapeBox::StaticClass()).LinkOnScope();
		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(SoftClassProp.Get(), &Src),
			FDcPropertyDatum(SoftClassProp.Get(), &Dest)
		));
	}

	{
		TLazyObjectPtr<UPackage> Src = FindObject<UPackage>(ANY_PACKAGE, TEXT("/Script/DataConfigTests"), true);
		TLazyObjectPtr<UPackage> Dest;

		auto LazyObjectProp = FDcPropertyBuilder::LazyObject(UPackage::StaticClass()).LinkOnScope();
		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(LazyObjectProp.Get(), &Src),
			FDcPropertyDatum(LazyObjectProp.Get(), &Dest)
		));
	}

	UDcTestDelegateClass1* DelegateObj = NewObject<UDcTestDelegateClass1>();

	{
		FDcTestDelegate1 Src;
		Src.BindDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnOne);

		FDcTestDelegate1 Dest;

		auto DelegateProp = FDcPropertyBuilder::Delegate(
			DelegateObj->FindFunction(TEXT("ReturnOne"))
		).LinkOnScope();

		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(DelegateProp.Get(), &Src),
			FDcPropertyDatum(DelegateProp.Get(), &Dest)
		));
	}

	{
		FDcTestDynMulticastCallback1 Src;
		Src.AddDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnNone);

		FDcTestDynMulticastCallback1 Dest;

		//	atm seems there's no easy way to get delegate signature function directly
		//	even though it's in generated code
		auto MulticastInlineProp = FDcPropertyBuilder::MulticastInlineDelegate(

			CastChecked<UDelegateFunction>(CastFieldChecked<FMulticastInlineDelegateProperty>(
				FDcTestStruct2::StaticStruct()->FindPropertyByName(TEXT("DynMulticastField"))
			)->SignatureFunction)

		).LinkOnScope();

		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(MulticastInlineProp.Get(), &Src),
			FDcPropertyDatum(MulticastInlineProp.Get(), &Dest)
		));
	}

	{
		UDcTestDelegateClass1* Src = NewObject<UDcTestDelegateClass1>();
		Src->SparseCallback1.AddDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnNone);

		UDcTestDelegateClass1* Dest = NewObject<UDcTestDelegateClass1>();
		auto MulticastSparseProp = FDcPropertyBuilder::MulticastSparseDelegate(

			CastChecked<USparseDelegateFunction>(CastFieldChecked<FMulticastSparseDelegateProperty>(
				UDcTestDelegateClass1::StaticClass()->FindPropertyByName(TEXT("SparseCallback1"))
			)->SignatureFunction)

		).LinkOnScope();

		UTEST_OK("Builder Roundtrip3", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(MulticastSparseProp.Get(), &Src),
			FDcPropertyDatum(MulticastSparseProp.Get(), &Dest)
		));
	}

	return true;
}


DC_TEST("DataConfig.Core.Property.SetMapWithHoles")
{
	FDcTestSetMapOfName Src;
	for (int Ix = 0; Ix < 20000; Ix++)
	{
		FString Str1 = FString::Printf(TEXT("<%d>"), Ix);
		FString Str2 = FString::Printf(TEXT("{%d}"), Ix);

		Src.NameSet.Add(FName(Str1));
		Src.NameStrMap.Add(FName(Str1), Str2);
	}

	for (int Ix = 0; Ix < 20000; Ix++)
	{
		FString Str1 = FString::Printf(TEXT("<%d>"), Ix);
		if (Ix % 7 == 0)
			Src.NameSet.Remove(FName(Str1));
		if (Ix % 13 == 0)
			Src.NameStrMap.Remove(FName(Str1));
	}

	FDcTestSetMapOfName Dest;
	UTEST_OK("Property Large Set Map", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(&Src),
		FDcPropertyDatum(&Dest)
	));

	return true;
}

