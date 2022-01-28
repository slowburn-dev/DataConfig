#include "DataConfig/Diagnostic/DcDiagnosticMsgPack.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDMsgPack
{
	
static FDcDiagnosticDetail _MsgPackDetails[] = {

	{ TypeByteMismatch, TEXT("MsgPack type byte mismatch, Expect '{0}', Actual '{1}'")},
	{ TypeByteMismatchNoExpect, TEXT("MsgPack type byte mismatch, Actual '{0}'")},
	{ UnexpectedArrayEnd, TEXT("Unexpected array end"), },
	{ UnexpectedMapEnd, TEXT("Unexpected map end"), },
	{ ReadPassArrayEnd, TEXT("Reading pass array end"), },
	{ ReadPassMapEnd, TEXT("Reading pass map end"), },

	//	Reader
	{ UnknownMsgTypeByte, TEXT("Unknown MsgPack type byte: 0x{0}"), },
	{ ReadingPastEnd, TEXT("Buffer already ended, can not read any value"), },
	{ SizeOverInt32Max, TEXT("Size over int32::max isn't supported yet."), },
	{ ArrayRemains, TEXT("Array ins't fully consumed on end, remains: {0}"), },
	{ MapRemains, TEXT("Map ins't fully consumed on end, remains: {0}"), },

};

FDcDiagnosticGroup Details = {
	Category,
	DcDimOf(_MsgPackDetails),
	_MsgPackDetails
};

} // namespace DcDMsgPack

