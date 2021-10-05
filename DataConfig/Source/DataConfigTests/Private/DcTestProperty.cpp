#include "DcTestProperty.h"
#include "DcTestCommon.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

int UDcTestDelegateClass1::ReturnOne(int Int) { return Int; }
void UDcTestDelegateClass1::ReturnNone(int Int) { /*pass*/ }

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

	UTEST_OK("FDcTestStruct1 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
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

	UTEST_OK("FDcTestStruct2 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStruct2 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	//	compile checks
	if (0)
	{
		FDcReader Reader;
		Reader.ReadWeakObjectField(&Source.WeakObjetField).Ok();
		Reader.ReadLazyObjectField(&Source.LazyObjectField).Ok();
		Reader.ReadSoftObjectField(&Source.SoftObjectField).Ok();
		Reader.ReadSoftClassField(&Source.SoftClassField).Ok();
		Reader.ReadInterfaceField(&Source.InterfaceField).Ok();
		Reader.ReadDelegate(&Source.DelegateField).Ok();
		Reader.ReadMulticastInlineDelegate(&Source.DynMulticastField).Ok();

		FDcWriter Writer;
		Writer.WriteWeakObjectField(Source.WeakObjetField).Ok();
		Writer.WriteLazyObjectField(Source.LazyObjectField).Ok();
		Writer.WriteSoftObjectField(Source.SoftObjectField).Ok();
		Writer.WriteSoftClassField(Source.SoftClassField).Ok();
		Writer.WriteInterfaceField(Source.InterfaceField).Ok();
		Writer.WriteDelegate(Source.DelegateField).Ok();
		Writer.WriteMulticastInlineDelegate(Source.DynMulticastField).Ok();
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

	UTEST_OK("UDcTestDelegateClass1 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
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

	UTEST_OK("FDcTestStruct3 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
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

	UTEST_OK("FDcTestStruct4 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
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

	UTEST_OK("FDcTestStruct_Blob roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}

DC_TEST("DataConfig.Core.Property.ScalarRoots")
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

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("BoolField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("BoolField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("NameField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("NameField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StringField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StringField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("TextField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("TextField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("EnumField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("EnumField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("FloatField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("FloatField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("DoubleField")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("DoubleField"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("Int8Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("Int8Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("Int16Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("Int16Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("Int32Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("Int32Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("Int64Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("Int64Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("UInt8Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("UInt8Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("UInt16Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("UInt16Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("UInt32Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("UInt32Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("UInt64Field")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("UInt64Field"))
	));

	UTEST_OK("Scalar Root Roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}


DC_TEST("DataConfig.Core.Property.ContainerRoots")
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

	UTEST_OK("Container Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StringArray")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StringArray"))
	));

	UTEST_OK("Container Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StringSet")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StringSet"))
	));

	UTEST_OK("Container Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StringMap")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StringMap"))
	));

	UTEST_OK("Container Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StructArray")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StructArray"))
	));

	UTEST_OK("Container Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StructSet")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StructSet"))
	));

	UTEST_OK("Container Root Roundtrip", DcTestPropertyRoundtrip(this,
		DcAutomationUtils::TryGetMemberDatum(SourceDatum, TEXT("StructMap")),
		DcAutomationUtils::TryGetMemberDatum(DestDatum, TEXT("StructMap"))
	));

	UTEST_OK("Container Root Roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}


