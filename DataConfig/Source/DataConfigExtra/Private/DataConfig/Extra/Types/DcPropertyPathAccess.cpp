#include "DataConfig/Extra/Types/DcPropertyPathAccess.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"

#include "DataConfig/Automation/DcAutomationUtils.h"

#include "Containers/StringView.h"

#include "PropertyPathHelpers.h"

namespace DcExtra {

void _SplitPath(FStringView InSv, FStringView& OutHead, FStringView& OutTail)
{
	int32 Ix;
	bool Found = InSv.FindChar(TCHAR('.'), Ix);
	if (!Found)
	{
		OutHead = InSv;
		OutTail = FStringView();
	}
	else
	{
		OutHead = InSv.Left(Ix);
		OutTail = InSv.RightChop(Ix + 1);
	}
}

FDcResult TraverseReaderByPath(FDcPropertyReader* Reader, const FString& Path)
{
	FStringView Remaining = Path;
	while (true)
	{
		FStringView Cur;
		FStringView Tail;
		_SplitPath(Remaining, Cur, Tail);

		EDcDataEntry Next;
		DC_TRY(Reader->PeekRead(&Next));

		if (Next == EDcDataEntry::StructRoot)
		{
			DC_TRY(Reader->ReadStructRoot(nullptr));
			while (true)
			{
				FName CurName(Cur);
				FName FieldName;
				DC_TRY(Reader->ReadName(&FieldName));

				if (FieldName == CurName)
				{
					//	found
					break;
				}
				else
				{
					//	move to next position
					DC_TRY(Reader->SkipRead());

					DC_TRY(Reader->PeekRead(&Next));
					if (Next == EDcDataEntry::StructEnd)
						return DC_FAIL(DcDReadWrite, CantFindPropertyByName) << Reader->FormatHighlight();
				}
			}
		}
		else if (Next == EDcDataEntry::ClassRoot)
		{
			FDcClassStat Stat;
			DC_TRY(Reader->ReadClassRoot(&Stat));

			if (Stat.Control != FDcClassStat::EControl::ExpandObject)
				return DC_FAIL(DcDExtra, ExpectClassExpand);

			while (true)
			{
				FName CurName(Cur);
				FName FieldName;
				DC_TRY(Reader->ReadName(&FieldName));

				if (FieldName == CurName)
				{
					//	found
					break;
				}
				else
				{
					//	move to next position
					DC_TRY(Reader->SkipRead());

					DC_TRY(Reader->PeekRead(&Next));
					if (Next == EDcDataEntry::ClassEnd)
						return DC_FAIL(DcDReadWrite, CantFindPropertyByName) << Reader->FormatHighlight();
				}
			}
		}
		else if (Next == EDcDataEntry::ArrayRoot)
		{
			return DcNoEntry();
		}
		else if (Next == EDcDataEntry::SetRoot)
		{
			return DcNoEntry();
		}
		else if (Next == EDcDataEntry::MapRoot)
		{
			return DcNoEntry();
		}

		if (Tail.IsEmpty())
			break;
		else
			Remaining = Tail;
	}

	return DcOk();
}

FDcResult GetDatumPropertyByPath(const FDcPropertyDatum& Datum, const FString& Path, FDcPropertyDatum& OutDatum)
{
	FDcPropertyReader Reader(Datum);
	FDcPropertyConfig Config = FDcPropertyConfig::MakeDefault();
	Config.ExpandObjectPredicate.BindLambda([](FObjectProperty* ObjProperty){
		return true;
	});
	DC_TRY(Reader.SetConfig(Config));
	DC_TRY(TraverseReaderByPath(&Reader, Path));

	FFieldVariant Property;
	DC_TRY(Reader.PeekReadProperty(&Property));

	if (Property.IsUObject())
		return DC_FAIL(DcDReadWrite, ExpectFieldButFoundUObject) << Property.GetClassName() << Property.GetFName();

	DC_TRY(Reader.ReadDataEntry(Property.ToFieldUnsafe()->GetClass(), OutDatum));

	return DcOk();
}

} // namespace DcExtra


DC_TEST("DataConfig.Extra.PathAccess.ReadByPath")
{
	using namespace DcExtra;

	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);

	UDcExtraTestClassOuter* Outer = NewObject<UDcExtraTestClassOuter>();
	Outer->StructRoot.Middle.InnerMost.StrField = TEXT("Foo");

	UTEST_TRUE("Extra PathAccess ReadByPath", GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField") == TEXT("Foo"));

	return true;
}

DC_TEST("DataConfig.Extra.PathAccess.PropertyPathHelpers")
{
	//	demonstrating UE4 PropertyPathHelpers

	UDcExtraTestClassOuter* Outer = NewObject<UDcExtraTestClassOuter>();
	Outer->StructRoot.Middle.InnerMost.StrField = TEXT("Foo");

	FString Str;
	UTEST_TRUE("Extra PropertyPathHelpers", PropertyPathHelpers::GetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), Str));
	UTEST_TRUE("Extra PropertyPathHelpers", Str == TEXT("Foo"));

	return true;
}

