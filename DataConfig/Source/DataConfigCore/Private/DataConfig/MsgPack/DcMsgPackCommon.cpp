#include "DataConfig/MsgPack/DcMsgPackCommon.h"

#include "DataConfig/DcCorePrivate.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"

namespace DcMsgPackCommon
{

void RecordTypeByteOffset(DcMsgPackUtils::FTypeByteQueue& Self, int Offset)
{
	using namespace DcMsgPackUtils;
	using EState = FTypeByteQueue::EState;

	int Index = Self.LastTypeByteHead;
	Self.LastTypeBytes[Index] = Offset;
	Self.LastTypeByteHead = (Index + 1) & (FTypeByteQueue::_SIZE - 1);

	switch (Self.State)
	{
		case EState::Empty:
		{
			Self.State = EState::NoFlippedYet;
			break;
		}
		case EState::NoFlippedYet:
		{
			if (Self.LastTypeByteHead == 0)
				Self.State = EState::Flipped;
			break;
		}
		case EState::Flipped:
		{
			//	pass
			break;
		}
	}
}

int GetOldestOffset(DcMsgPackUtils::FTypeByteQueue& Self)
{
	using namespace DcMsgPackUtils;
	using EState = FTypeByteQueue::EState;

	switch (Self.State)
	{
		case EState::Empty:
			return INDEX_NONE;
		case EState::NoFlippedYet:
			return Self.LastTypeBytes[0];
		case EState::Flipped:
			return Self.LastTypeBytes[(Self.LastTypeByteHead + 1) & (FTypeByteQueue::_SIZE - 1)];
	}

	return INDEX_NONE;
}

FString FormatMsgPackHighlight(FDcBlobViewData Blob, int End, const TCHAR* Header, const TCHAR* Tail)
{
	if (End <= 0)
		return TEXT("<empty>");

	TDcStoreThenReset<bool> ScopedCanFail(DcEnv().bExpectFail, true);

	FDcMsgPackReader SubReader(Blob);
	DcCorePrivate::FStringNewlineDevice Output;
	FDcPrettyPrintWriter Writer(Output);

	FDcPipeVisitor PipeVisitor{&SubReader, &Writer};
	PipeVisitor.PreVisit.BindLambda(
	[&](FDcPipeVisitor* Visitor, EPipeVisitControl& OutControl) -> FDcResult
	{
		if (SubReader.State.Index >= End)
			OutControl = EPipeVisitControl::BreakVisit;
		return DcOk();
	});

	FDcResult Result = PipeVisitor.PipeVisit();

	TStringBuilder<1024> Sb;
	Sb.Append(Header);
	Sb << TCHAR('\n');
	Sb.Append(*Output);
	Sb.Append(Tail);
	if (!Result.Ok())
		Sb.Append(TEXT(" (... error during read)"));

	return Sb.ToString();
}

} // namespace DcMsgPackCommon
