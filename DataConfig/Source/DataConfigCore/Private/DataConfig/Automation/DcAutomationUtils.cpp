#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"

namespace DcAutomationUtils
{

FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum)
{
	FDcPropertyReader LhsReader(LhsDatum);
	FDcPropertyReader RhsReader(RhsDatum);

	while (true)
	{
		EDcDataEntry Next;
		EDcDataEntry RhsPeekEntry;
		DC_TRY(LhsReader.PeekRead(&Next));
		DC_TRY(RhsReader.PeekRead(&RhsPeekEntry));

		DC_TRY(DcExpect(Next == RhsPeekEntry));

		if (Next == EDcDataEntry::Ended)
		{
			return DcOk();
		}
		if (Next == EDcDataEntry::Nil)
		{
			DC_TRY(LhsReader.ReadNil());
			DC_TRY(RhsReader.ReadNil());
		}
		else if (Next == EDcDataEntry::Bool)
		{
			bool Lhs;
			DC_TRY(LhsReader.ReadBool(&Lhs));

			bool Rhs;
			DC_TRY(RhsReader.ReadBool(&Rhs));

			DC_TRY(DcExpect(Lhs == Rhs));
		}
		else if (Next == EDcDataEntry::Name)
		{
			FName Lhs;
			DC_TRY(LhsReader.ReadName(&Lhs));

			FName Rhs;
			DC_TRY(RhsReader.ReadName(&Rhs));

			DC_TRY(DcExpect(Lhs == Rhs));
		}
		else if (Next == EDcDataEntry::String)
		{
			FString Lhs;
			DC_TRY(LhsReader.ReadString(&Lhs));

			FString Rhs;
			DC_TRY(RhsReader.ReadString(&Rhs));

			DC_TRY(DcExpect(Lhs == Rhs));
		}
		else if (Next == EDcDataEntry::Text)
		{
			FText Lhs;
			DC_TRY(LhsReader.ReadText(&Lhs));

			FText Rhs;
			DC_TRY(RhsReader.ReadText(&Rhs));

			//DC_TRY(DcExpect(Lhs == Rhs));
		}
		else
		{
			return DcNoEntry();
		}
	}

	return DcOk();
}


}	// namespace DcAutomationUtils
