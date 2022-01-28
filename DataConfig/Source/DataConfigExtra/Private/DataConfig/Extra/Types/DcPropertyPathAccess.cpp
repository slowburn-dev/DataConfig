#include "DataConfig/Extra/Types/DcPropertyPathAccess.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

#include "Misc/CString.h"
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
			FFieldVariant StructField;
			DC_TRY(Reader->PeekReadProperty(&StructField));
			UStruct* Struct = DcPropertyUtils::TryGetStruct(StructField);
			check(Struct);
			
			DC_TRY(Reader->ReadStructRoot());
			while (true)
			{
				FName CurName(Cur);
				FName FieldName;
				DC_TRY(Reader->ReadName(&FieldName));

				if (FProperty* Property = DcPropertyUtils::FindEffectivePropertyByName(Struct, CurName))
				{
					if (Property->GetFName() == FieldName)
					{
						//	found
						break;
					}
				}

				//	move to next position
				DC_TRY(Reader->SkipRead());

				DC_TRY(Reader->PeekRead(&Next));
				if (Next == EDcDataEntry::StructEnd)
					return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
						<< Cur << Reader->FormatHighlight();
			}
		}
		else if (Next == EDcDataEntry::ClassRoot)
		{
			FFieldVariant ClassField;
			DC_TRY(Reader->PeekReadProperty(&ClassField));
			UStruct* Struct = DcPropertyUtils::TryGetStruct(ClassField);
			check(Struct);
			
			FDcClassAccess Access;
			DC_TRY(Reader->ReadClassRootAccess(Access));

			if (Access.Control != FDcClassAccess::EControl::ExpandObject)
				return DC_FAIL(DcDExtra, ExpectClassExpand);

			while (true)
			{
				FName CurName(Cur);
				FName FieldName;
				DC_TRY(Reader->ReadName(&FieldName));

				if (FProperty* Property = DcPropertyUtils::FindEffectivePropertyByName(Struct, CurName))
				{
					if (Property->GetFName() == FieldName)
					{
						//	found
						break;
					}
				}

				//	move to next position
				DC_TRY(Reader->SkipRead());

				DC_TRY(Reader->PeekRead(&Next));
				if (Next == EDcDataEntry::ClassEnd)
					return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
						<< Cur << Reader->FormatHighlight();
			}
		}
		else if (Next == EDcDataEntry::ArrayRoot)
		{
			DC_TRY(Reader->ReadArrayRoot());
			int Index = FCString::Strtoi(Cur.begin(), nullptr, 10);

			while (Index > 0)
			{
				DC_TRY(Reader->SkipRead());
				--Index;
			}
		}
		else if (Next == EDcDataEntry::SetRoot)
		{
			DC_TRY(Reader->ReadSetRoot());
			int Index = FCString::Strtoi(Cur.begin(), nullptr, 10);

			while (Index > 0)
			{
				DC_TRY(Reader->SkipRead());
				--Index;
			}
		}
		else if (Next == EDcDataEntry::MapRoot)
		{
			DC_TRY(Reader->ReadMapRoot());

			FName CurName(Cur);
			while (true)
			{
				EDcDataEntry MapNext;
				DC_TRY(Reader->PeekRead(&MapNext));

				if (MapNext == EDcDataEntry::String)
				{
					FString FieldStr;
					DC_TRY(Reader->ReadString(&FieldStr));

					if (FieldStr == Cur)
						break;	// found
				}
				else if (MapNext == EDcDataEntry::Name)
				{
					FName FieldName;
					DC_TRY(Reader->ReadName(&FieldName));

					if (FieldName == CurName)
						break;	// found
				}
				else
				{
					return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
						<< EDcDataEntry::Name << EDcDataEntry::String << MapNext << Reader->FormatHighlight();
				}
			}
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


DC_TEST("DataConfig.Extra.PathAccess.ReadWriteByPath")
{
	using namespace DcExtra;

	UDcExtraTestClassOuter* Outer = NewObject<UDcExtraTestClassOuter>();
	Outer->StructRoot.Middle.InnerMost.StrField = TEXT("Foo");
	Outer->StructRoot.Arr.Emplace(FDcExtraTestStructNestInnerMost{TEXT("Bar0")});
	Outer->StructRoot.Arr.Emplace(FDcExtraTestStructNestInnerMost{TEXT("Bar1")});
	Outer->StructRoot.NameMap.Emplace(TEXT("FooKey"), FDcExtraTestStructNestInnerMost{TEXT("FooValue")});
	Outer->StructRoot.Middle.InnerMost.ObjField = Outer;

	auto CheckStrPtr = [](FString* StrPtr, const TCHAR* Expect)
	{
		if (!StrPtr)
			return false;

		return *StrPtr == Expect;
	};

	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField"), TEXT("Foo")));
	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField"), TEXT("Bar0")));
	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField"), TEXT("Bar1")));
	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField"), TEXT("FooValue")));

	UTEST_TRUE("Extra PathAccess WriteByPath", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField", TEXT("AltFoo")));
	UTEST_TRUE("Extra PathAccess WriteByPath", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField", TEXT("AltBar0")));
	UTEST_TRUE("Extra PathAccess WriteByPath", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField", TEXT("AltBar1")));
	UTEST_TRUE("Extra PathAccess WriteByPath", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField", TEXT("AltFooValue")));

	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField"), TEXT("AltFoo")));
	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField"), TEXT("AltBar0")));
	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField"), TEXT("AltBar1")));
	UTEST_TRUE("Extra PathAccess ReadByPath", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField"), TEXT("AltFooValue")));

	UTEST_TRUE("Extra PathAccess ReadByPath", GetDatumPropertyByPath<FDcExtraTestStructNestMiddle>(FDcPropertyDatum(Outer), "StructRoot.Middle") == &Outer->StructRoot.Middle);
	UTEST_TRUE("Extra PathAccess ReadByPath", GetDatumPropertyByPath<UDcExtraTestClassOuter>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.ObjField") == Outer->StructRoot.Middle.InnerMost.ObjField);
	
	return true;
}

DC_TEST("DataConfig.Extra.PathAccess.PropertyPathHelpers")
{
	//	demo UE4 PropertyPathHelpers read and write
	UDcExtraTestClassOuter* Outer = NewObject<UDcExtraTestClassOuter>();
	Outer->StructRoot.Middle.InnerMost.StrField = TEXT("Foo");

	FString Str;
	UTEST_TRUE("Extra PropertyPathHelpers", PropertyPathHelpers::GetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), Str));
	UTEST_TRUE("Extra PropertyPathHelpers", Str == TEXT("Foo"));

	UTEST_TRUE("Extra PropertyPathHelpers", PropertyPathHelpers::SetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), FString(TEXT("Bar"))));
	UTEST_TRUE("Extra PropertyPathHelpers", Outer->StructRoot.Middle.InnerMost.StrField == TEXT("Bar"));

	FDcExtraTestStructNestMiddle CopiedMiddle;
	UTEST_TRUE("Extra PropertyPathHelpers", PropertyPathHelpers::GetPropertyValue(Outer, TEXT("StructRoot.Middle"), CopiedMiddle));
	//	note that PropertyPathHelpers::GetPropertyValue creates a copy
	UTEST_TRUE("Extra PropertyPathHelpers", &CopiedMiddle != &Outer->StructRoot.Middle);
	
	return true;
}
