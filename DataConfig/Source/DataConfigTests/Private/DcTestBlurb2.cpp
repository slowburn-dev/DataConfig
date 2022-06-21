#include "DcTestBlurb2.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Json/DcJsonReader.h"

void FDcTestExample2::MakeFixture()
{
	StrField = TEXT("Foo");
	StrArrField[0] = TEXT("One");
	StrArrField[1] = TEXT("Two");
	StrArrField[2] = TEXT("Three");

	UDcShapeSquare* Square = NewObject<UDcShapeSquare>();
	Square->ShapeName = TEXT("MyBox");
	Square->Radius = 2;
	InlineField = Square;

	RefField = NewObject<UDcTestClass1>();
}

DC_TEST("DataConfig.Core.Blurb.ArrayRoot")
{
	UTEST_OK("Blurb ArrayRoot", []{

		FString Fixture = TEXT("[1,2,3,4,5]");
		TArray<int> Arr;

		//	create int array property
		using namespace DcPropertyUtils;
		auto ArrProp = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::Int()
			).LinkOnScope();

		FDcJsonReader Reader{Fixture};
		DC_TRY(DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(ArrProp.Get(), &Arr)));

		//	validate results
		check(Arr.Num() == 5);
		check(Arr[4] == 5);

		return DcOk();

	}());

	return true;
}

DC_TEST("DataConfig.Core.Blurb.Property")
{
	UTEST_OK("Blurb Property", []{

		FDcTestExample2 Value;
		Value.MakeFixture();

		//	construct the reader
		FDcPropertyDatum Datum(&Value);
		FDcPropertyReader Reader(Datum);
		//DcAutomationUtils::DumpToLog(&Reader);
		DcAutomationUtils::DumpFormat(&Reader);

		return DcOk();

	}());


	return true;
}

