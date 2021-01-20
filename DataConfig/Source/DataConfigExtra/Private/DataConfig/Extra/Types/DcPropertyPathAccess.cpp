#include "DataConfig/Extra/Types/DcPropertyPathAccess.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Property/DcPropertyReader.h"

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
			while (true)
			{
				FName CurName(Cur);
				FName FieldName;
				DC_TRY(Reader->ReadName(&FieldName));

				if (FieldName == CurName)
				{
					break;
				}
				else
				{
					//	TODO we dont' have this atm...
					//	Reader->SkipRead();
				}
			}
		}
		else if (Next == EDcDataEntry::ClassRoot)
		{
		}
		else if (Next == EDcDataEntry::ArrayRoot)
		{
		}
		else if (Next == EDcDataEntry::SetRoot)
		{
		}
		else if (Next == EDcDataEntry::MapRoot)
		{
		}


		//UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *FString(Cur));

		if (Tail.IsEmpty())
			break;
		else
			Remaining = Tail;
	}

	return DcOk();
}

} // namespace DcExtra


DC_TEST("DataConfig.Extra.PathAccess.Read")
{
	using namespace DcExtra;

	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);

	//TraverseReaderByPath(nullptr, TEXT("Foo.Bar.Baz.Bart"));

	return true;
}


DC_TEST("DataConfig.Extra.PathAccess.PropertyPathHelpers")
{
	using namespace DcExtra;


	return true;
}

