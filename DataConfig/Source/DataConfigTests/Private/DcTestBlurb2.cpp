#include "DcTestBlurb2.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"

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


DC_TEST("DataConfig.Core.Blurb.Coercion")
{
	UTEST_OK("Blurb Coercion", []{

		FDcJsonReader Reader(TEXT(R"(
			1.234567
		)"));

		//	check coercion
		//	note this is a query and doesn't affect reading at all
		bool bCanCoerceToStr;
		DC_TRY(Reader.Coercion(EDcDataEntry::String, &bCanCoerceToStr));
		check(bCanCoerceToStr);

		//	read number token as stjring
		//	note here we skipped parsing the number to float
		FString NumStr;
		DC_TRY(Reader.ReadString(&NumStr));

		check(NumStr == TEXT("1.234567"));
		return DcOk();

	}());

	return true;
}
