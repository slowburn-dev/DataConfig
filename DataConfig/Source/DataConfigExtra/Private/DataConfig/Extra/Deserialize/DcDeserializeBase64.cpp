#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/DcEnv.h"

namespace DcExtra
{

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx)
{
	return EDcDeserializePredicateResult::Pass;
}

DATACONFIGEXTRA_API FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return DcOkWithProcessed(OutRet);
}

}	//	namespace DcExtra

