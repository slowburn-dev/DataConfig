#include "DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcPropertyHandlers {

FDcResult DcPropertyHandlers::HandlerScalarDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer);
	return DcOkWithProcessed(OutRet);
}


}	//	namespace DcPropertyHandlers


