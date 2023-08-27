#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcDeserializeUtils
{

FDcResult RecursiveDeserialize(FDcDeserializeContext& Ctx)
{
	FFieldVariant Property;
	DC_TRY(Ctx.Writer->PeekWriteProperty(&Property));
	Ctx.Properties.Push(Property);

	DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

	FFieldVariant Popped = Ctx.Properties.Pop();
	if (Property != Popped)
		return DC_FAIL(DcDSerDe, RecursiveDeserializeTopPropertyChanged);

	return DcOk();
}

EDcDeserializePredicateResult PredicateIsRootProperty(FDcDeserializeContext& Ctx)
{
	check(Ctx.Properties.Num() > 0);
    return Ctx.TopProperty() == Ctx.Properties[0]
        ? EDcDeserializePredicateResult::Process
        : EDcDeserializePredicateResult::Pass;
}

} // namespace DcDeserializeUtils



