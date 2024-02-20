#include "DcTestProperty3.h"
#include "DcTestProperty.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

#if WITH_EDITORONLY_DATA
DC_TEST("DataConfig.Core.Property.DcMetaSkip")
{

	DcAutomationUtils::AmendMetaData(FDcTestMeta1::StaticStruct(), TEXT("SkipField1"), TEXT("DcSkip"), TEXT(""));

	FDcTestMeta1 Source;
	Source.SkipField1 = 253;

	{
		FDcPropertyReader Reader{FDcPropertyDatum(&Source)};

		UTEST_OK("DcMetaSkip", Reader.ReadStructRoot());
		UTEST_OK("DcMetaSkip", Reader.ReadStructEnd());
	}

	{
		FDcPropertyWriter Writer{FDcPropertyDatum(&Source)};

		UTEST_OK("DcMetaSkip", Writer.WriteStructRoot());
		UTEST_DIAG("DcMetaSkip", Writer.WriteName(TEXT("SkipField1")), DcDReadWrite, CantFindPropertyByName);
	}

	return true;
}
#endif // WITH_EDITORONLY_DATA

DC_TEST("DataConfig.Core.Property.ClassAccessControl")
{
	FDcTestClassAccessControl Source;
	FDcPropertyDatum SourceDatum(&Source);
	{
		UDcShapeBox* Box = NewObject<UDcShapeBox>();
		Box->ShapeName = TEXT("MyBox");
		Box->Height = 12;
		Box->Width = 34;
		Source.InlineField = Box;

		UDcTestClass1* Obj = NewObject<UDcTestClass1>();
		Obj->IntField = 56;
		Obj->StrField = "Foo";
		Source.RefField = Obj;
	}

	auto _SkipReadFieldsN = [](FDcPropertyReader* Reader, int N) -> FDcResult
	{
		for (int Ix = 0; Ix < N; Ix++)
		{
			FName Name;
			DC_TRY(Reader->ReadName(&Name));
			DC_TRY(Reader->SkipRead());
		}
		return DcOk();
	};

	{
		FDcPropertyReader Reader{SourceDatum};

		UTEST_OK("ClassAccessControl", Reader.ReadStructRoot());

		{
			//	default expand inline
			FName Name;
			UTEST_OK("ClassAccessControl", Reader.ReadName(&Name));
			UTEST_TRUE("ClassAccessControl", Name == TEXT("InlineField"));

			UTEST_OK("ClassAccessControl", Reader.ReadClassRoot());
			UTEST_OK("ClassAccessControl", _SkipReadFieldsN(&Reader, 3));
			UTEST_OK("ClassAccessControl", Reader.ReadClassEnd());
		}

		{
			//	default reference
			FName Name;
			UTEST_OK("ClassAccessControl", Reader.ReadName(&Name));
			UTEST_TRUE("ClassAccessControl", Name == TEXT("RefField"));

			UTEST_OK("ClassAccessControl", Reader.ReadClassRoot());
			UObject* Ref;
			UTEST_OK("ClassAccessControl", Reader.ReadObjectReference(&Ref));
			UTEST_TRUE("ClassAccessControl", Ref == Source.RefField);
			UTEST_OK("ClassAccessControl", Reader.ReadClassEnd());
		}
	}

	{
		FDcPropertyReader Reader{SourceDatum};

		UTEST_OK("ClassAccessControl", Reader.ReadStructRoot());

		{
			//	force ref
			FName Name;
			UTEST_OK("ClassAccessControl", Reader.ReadName(&Name));
			UTEST_TRUE("ClassAccessControl", Name == TEXT("InlineField"));

			FDcClassAccess Access{FDcClassAccess::EControl::ReferenceOrNone};
			UTEST_OK("ClassAccessControl", Reader.ReadClassRootAccess(Access));

			UObject* Ref;
			UTEST_OK("ClassAccessControl", Reader.ReadObjectReference(&Ref));
			UTEST_TRUE("ClassAccessControl", Ref == Source.InlineField);

			UTEST_OK("ClassAccessControl", Reader.ReadClassEndAccess(Access));
		}

		{
			//	force expand
			FName Name;
			UTEST_OK("ClassAccessControl", Reader.ReadName(&Name));
			UTEST_TRUE("ClassAccessControl", Name == TEXT("RefField"));

			FDcClassAccess Access{FDcClassAccess::EControl::ExpandObject};
			UTEST_OK("ClassAccessControl", Reader.ReadClassRootAccess(Access));
			UTEST_OK("ClassAccessControl", _SkipReadFieldsN(&Reader, 2));
			UTEST_OK("ClassAccessControl", Reader.ReadClassEndAccess(Access));
		}
	}

	auto _PipeInnerClassDatum = [](FDcPropertyWriter* Writer, FDcPropertyDatum Datum) -> FDcResult
	{
		FDcPropertyReader Reader{Datum};
		DC_TRY(Reader.ReadClassRoot());

		FDcPipeVisitor PipeVisitor(&Reader, Writer);
		PipeVisitor.PeekVisit.BindLambda(
		[](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl) -> FDcResult
		{
			if (Next == EDcDataEntry::ClassEnd)
				OutControl = EPipeVisitControl::BreakVisit;
			return DcOk();
		});
		return PipeVisitor.PipeVisit();
	};

	{
		FDcTestClassAccessControl Dest;
		FDcPropertyDatum DestDatum(&Dest);
		Dest.InlineField = NewObject<UDcShapeBox>();
		FDcPropertyWriter Writer{DestDatum};

		UTEST_OK("ClassAccessControl", Writer.WriteStructRoot());

		{
			//	default expand inline
			UTEST_OK("ClassAccessControl", Writer.WriteName(TEXT("InlineField")));
			UTEST_OK("ClassAccessControl", Writer.WriteClassRoot());
			UTEST_OK("ClassAccessControl", _PipeInnerClassDatum(&Writer, FDcPropertyDatum(Source.InlineField)));
			UTEST_OK("ClassAccessControl", Writer.WriteClassEnd());
		}

		{
			//	default reference
			UTEST_OK("ClassAccessControl", Writer.WriteName(TEXT("RefField")));
			UTEST_OK("ClassAccessControl", Writer.WriteClassRoot());
			UTEST_OK("ClassAccessControl", Writer.WriteObjectReference(Source.RefField));
			UTEST_OK("ClassAccessControl", Writer.WriteClassEnd());
		}

		UTEST_OK("ClassAccessControl", Writer.WriteStructEnd());
		UTEST_OK("ClassAccessControl", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	}

	{
		FDcTestClassAccessControl Dest;
		FDcPropertyDatum DestDatum(&Dest);
		Dest.RefField = NewObject<UDcTestClass1>();
		FDcPropertyWriter Writer{DestDatum};

		UTEST_OK("ClassAccessControl", Writer.WriteStructRoot());

		{
			//	force ref
			UTEST_OK("ClassAccessControl", Writer.WriteName(TEXT("InlineField")));
			FDcClassAccess Access{FDcClassAccess::EControl::ReferenceOrNone};
			UTEST_OK("ClassAccessControl", Writer.WriteClassRootAccess(Access));
			UTEST_OK("ClassAccessControl", Writer.WriteObjectReference(Source.InlineField));
			UTEST_OK("ClassAccessControl", Writer.WriteClassEndAccess(Access));
		}

		{
			//	force expand
			UTEST_OK("ClassAccessControl", Writer.WriteName(TEXT("RefField")));
			FDcClassAccess Access{FDcClassAccess::EControl::ExpandObject};
			UTEST_OK("ClassAccessControl", Writer.WriteClassRootAccess(Access));
			UTEST_OK("ClassAccessControl", _PipeInnerClassDatum(&Writer, FDcPropertyDatum(Source.RefField)));
			UTEST_OK("ClassAccessControl", Writer.WriteClassEndAccess(Access));
		}

		UTEST_OK("ClassAccessControl", Writer.WriteStructEnd());
		UTEST_OK("ClassAccessControl", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum, DcAutomationUtils::EReadDatumEqualType::ExpandAllObjects));
	}

	return true;
}


DC_TEST("DataConfig.Core.Property.DiagNullObject")
{
	FDcTestStruct4 Dest;
	FDcPropertyWriter Writer{FDcPropertyDatum(&Dest)};

	UTEST_OK("DiagNullObject", Writer.WriteStructRoot());
	UTEST_OK("DiagNullObject", Writer.WriteName(TEXT("NormalObjectField1")));
	UTEST_OK("DiagNullObject", Writer.WriteClassRoot());
	UTEST_DIAG("DiagNullObject", Writer.WriteObjectReference(nullptr), DcDReadWrite, WriteObjectReferenceDoNotAcceptNull);

	return true;
}

