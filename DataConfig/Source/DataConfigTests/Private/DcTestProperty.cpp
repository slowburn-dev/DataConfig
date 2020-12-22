#include "DcTestCommon.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

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

	return true;
}
