#include "DcTestCommon.h"
#include "DcTestProperty.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

int UDcTestDelegateClass1::ReturnOne(int Int) { return Int; }
void UDcTestDelegateClass1::ReturnNone(int Int) { /*pass*/ }

static FDcResult _DcPropertyRoundtrip(FAutomationTestBase* Fixture, FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum)
{
	FDcPropertyReader Reader(FromDatum);
	FDcPropertyWriter Writer(ToDatum);
	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	return RoundtripVisit.PipeVisit();
}

DC_TEST("DataConfig.Core.Property.Primitive1")
{
	FDcTestStruct1 Source;

	Source.BoolField = true;
	Source.NameField = TEXT("SomeName");
	Source.StringField = TEXT("Some String");
	Source.TextField = FText(NSLOCTEXT("DcAdhocs", "T1", "Some Text"));
	Source.EnumField = EDcTestEnum1::Bar;

	Source.FloatField = 12.3f;
	Source.DoubleField = 23.4;

	Source.Int8Field = 123;
	Source.Int16Field = 234;
	Source.Int32Field = 34567;
	Source.Int64Field = 456789;

	Source.UInt8Field = -123;
	Source.UInt16Field = -234;
	Source.UInt32Field = -34567;
	Source.UInt64Field = -456789;

	FDcTestStruct1 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStruct1::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStruct1::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStruct1 roundtrip", _DcPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStruct1 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}

DC_TEST("DataConfig.Core.Property.Primitive2")
{
	FDcTestStruct2 Source;
	UDcTestDelegateClass1* DelegateObj = NewObject<UDcTestDelegateClass1>();
	UObject* InterfaceObj = NewObject<UDcTestInterface1Beta>();

	Source.ClassField = UDcTestDelegateClass1::StaticClass();
	Source.WeakObjetField = DelegateObj;
	Source.LazyObjectField = DelegateObj;
	Source.SoftObjectField = DelegateObj;
	Source.SoftClassField = UDcTestDelegateClass1::StaticClass();
	Source.InterfaceField = InterfaceObj;
	Source.FieldPathField = DcPropertyUtils::FirstEffectiveProperty(FDcTestStruct1::StaticStruct()->PropertyLink);

	Source.DelegateField.BindDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnOne);
	Source.DynMulticastField.AddDynamic(DelegateObj, &UDcTestDelegateClass1::ReturnNone);

	FDcTestStruct2 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStruct2::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStruct2::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStruct2 roundtrip", _DcPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStruct2 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	//	compile checks
	if (0)
	{
		FDcReader Reader;
		Reader.ReadWeakObjectField(&Source.WeakObjetField);
		Reader.ReadLazyObjectField(&Source.LazyObjectField);
		Reader.ReadSoftObjectField(&Source.SoftObjectField);
		Reader.ReadSoftClassField(&Source.SoftClassField);
		Reader.ReadInterfaceField(&Source.InterfaceField);
		Reader.ReadDelegate(&Source.DelegateField);
		Reader.ReadMulticastInlineDelegate(&Source.DynMulticastField);

		FDcWriter Writer;
		Writer.WriteWeakObjectField(Source.WeakObjetField);
		Writer.WriteLazyObjectField(Source.LazyObjectField);
		Writer.WriteSoftObjectField(Source.SoftObjectField);
		Writer.WriteSoftClassField(Source.SoftClassField);
		Writer.WriteInterfaceField(Source.InterfaceField);
		Writer.WriteDelegate(Source.DelegateField);
		Writer.WriteMulticastInlineDelegate(Source.DynMulticastField);
	}

	return true;
}

DC_TEST("DataConfig.Core.Property.Primitive_SparseDelegate")
{
	UDcTestDelegateClass1* Source = NewObject<UDcTestDelegateClass1>();

	Source->SparseCallback1.AddDynamic(Source, &UDcTestDelegateClass1::ReturnNone);

	UDcTestDelegateClass1* Dest = NewObject<UDcTestDelegateClass1>();
	UTEST_FALSE("Sparse delegate was not bound", Dest->SparseCallback1.IsBound());

	FDcPropertyDatum SourceDatum(UDcTestDelegateClass1::StaticClass(), Source);
	FDcPropertyDatum DestDatum(UDcTestDelegateClass1::StaticClass(), Dest);

	UTEST_OK("UDcTestDelegateClass1 roundtrip", _DcPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("UDcTestDelegateClass1 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	UTEST_TRUE("Sparse delegate is bound", Dest->SparseCallback1.IsBound());

	Dest->SparseCallback1.Clear();
	UTEST_FALSE("Sparse delegate was not bound", Dest->SparseCallback1.IsBound());

	UDcTestDelegateClass1* Tmp = NewObject<UDcTestDelegateClass1>();

	FDcPropertyReader Reader(SourceDatum);
	FDcPropertyWriter Writer(DestDatum);

	UTEST_OK("Try ReadSparseDelegateField", Reader.ReadClassRoot(nullptr));
	UTEST_OK("Try WriteSparseDelegateField", Writer.WriteClassRoot(FDcClassStat{FName(), FDcClassStat::EControl::ExpandObject}));

	UTEST_OK("Try ReadSparseDelegateField", Reader.ReadName(nullptr));
	UTEST_OK("Try WriteSparseDelegateField", Writer.WriteName(TEXT("SparseCallback1")));

	UTEST_OK("Try ReadSparseDelegateField", Reader.ReadSparseDelegateField(&Tmp->SparseCallback1));
	UTEST_OK("Try WriteSparseDelegateField", Writer.WriteSparseDelegateField(Tmp->SparseCallback1));

	UTEST_TRUE("Sparse delegate is bound", Dest->SparseCallback1.IsBound());

	return true;
}

DC_TEST("DataConfig.Core.Property.Containers")
{
	FDcTestStruct3 Source;

	Source.StringArray.Add(TEXT("Foo"));
	Source.StringArray.Add(TEXT("Bar"));
	Source.StringArray.Add(TEXT("Baz"));

	Source.StringSet.Add(TEXT("Foo"));
	Source.StringSet.Add(TEXT("Bar"));
	Source.StringSet.Add(TEXT("Baz"));

	Source.StringMap.Add(TEXT("1"), TEXT("One"));
	Source.StringMap.Add(TEXT("2"), TEXT("Two"));
	Source.StringMap.Add(TEXT("3"), TEXT("Three"));

	Source.StructArray.Add({TEXT("One"), 1});
	Source.StructArray.Add({TEXT("Two"), 2});
	Source.StructArray.Add({TEXT("Three"), 3});

	Source.StructSet.Add({TEXT("One"), 1});
	Source.StructSet.Add({TEXT("Two"), 2});
	Source.StructSet.Add({TEXT("Three"), 3});

	Source.StructMap.Add({TEXT("One"), 1}, {TEXT("Uno"), 1});
	Source.StructMap.Add({TEXT("Two"), 2}, {TEXT("Dos"), 2});
	Source.StructMap.Add({TEXT("Three"), 3}, {TEXT("Tres"), 3});

	FDcTestStruct3 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStruct3::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStruct3::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStruct3 roundtrip", _DcPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStruct3 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}


DC_TEST("DataConfig.Core.Property.ObjectReference")
{
	FDcTestStruct4 Source;

	UDcShapeBox* Box = NewObject<UDcShapeBox>();
	Box->ShapeName = TEXT("MyBox");
	Box->Height = 12.3f;
	Box->Width = 23.4f;

	Source.InlineObjectField1 = Box;
	Source.InlineObjectField2 = nullptr;

	Source.NormalObjectField1 = NewObject<UDcTestClass1>();
	Source.NormalObjectField1->IntField = 123;
	Source.NormalObjectField1->StrField = TEXT("Foo");
	Source.NormalObjectField2 = nullptr;

	FDcTestStruct4 Dest;

	// need to create inline obeject out side reader/writer
	Dest.InlineObjectField1 = NewObject<UDcShapeBox>();
	Dest.InlineObjectField2 = nullptr;

	FDcPropertyDatum SourceDatum(FDcTestStruct4::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStruct4::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStruct4 roundtrip", _DcPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStruct4 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}


DC_TEST("DataConfig.Core.Property.Blob")
{
	FDcTestStruct_Blob Source;

	Source.BlobField1.Add(1);
	Source.BlobField1.Add(2);
	Source.BlobField1.Add(3);

	Source.BlobField2.Add(253);
	Source.BlobField2.Add(254);
	Source.BlobField2.Add(255);

	FDcTestStruct_Blob Dest;

	FDcPropertyDatum SourceDatum(FDcTestStruct_Blob::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStruct_Blob::StaticStruct(), &Dest);

	FDcPropertyReader Reader(SourceDatum);
	FDcPropertyWriter Writer(DestDatum);

	{
		//	manual roundtrip
		UTEST_OK("FDcTestStruct_Blob roundtrip", Reader.ReadStructRoot(nullptr));
		UTEST_OK("FDcTestStruct_Blob roundtrip", Writer.WriteStructRoot(FDcStructStat{}));

		{
			UTEST_OK("FDcTestStruct_Blob roundtrip", Reader.ReadName(nullptr));
			UTEST_OK("FDcTestStruct_Blob roundtrip", Writer.WriteName(TEXT("BlobField1")));

			FDcBlobViewData BlobView;
			UTEST_TRUE("FDcTestStruct_Blob roundtrip", Reader.Coercion(EDcDataEntry::Blob));
			UTEST_OK("FDcTestStruct_Blob roundtrip", Reader.ReadBlob(&BlobView));

			bool bWriteOk;
			UTEST_TRUE("FDcTestStruct_Blob roundtrip", Writer.PeekWrite(EDcDataEntry::Blob, &bWriteOk).Ok() && bWriteOk);
			UTEST_OK("FDcTestStruct_Blob roundtrip", Writer.WriteBlob(BlobView));
		}

		{
			UTEST_OK("FDcTestStruct_Blob roundtrip", Reader.ReadName(nullptr));
			UTEST_OK("FDcTestStruct_Blob roundtrip", Writer.WriteName(TEXT("BlobField2")));

			FDcBlobViewData BlobView;
			UTEST_TRUE("FDcTestStruct_Blob roundtrip", Reader.Coercion(EDcDataEntry::Blob));
			UTEST_OK("FDcTestStruct_Blob roundtrip", Reader.ReadBlob(&BlobView));

			bool bWriteOk;
			UTEST_TRUE("FDcTestStruct_Blob roundtrip", Writer.PeekWrite(EDcDataEntry::Blob, &bWriteOk).Ok() && bWriteOk);
			UTEST_OK("FDcTestStruct_Blob roundtrip", Writer.WriteBlob(BlobView));
		}
	}

	UTEST_OK("FDcTestStruct4 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}

