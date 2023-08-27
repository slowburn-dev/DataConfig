#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Serialize/DcSerializer.h"

namespace DcSerializeUtils
{
	
FDcResult RecursiveSerialize(FDcSerializeContext& Ctx)
{
	FFieldVariant Property;
	DC_TRY(Ctx.Reader->PeekReadProperty(&Property));
	Ctx.Properties.Push(Property);

	DC_TRY(Ctx.Serializer->Serialize(Ctx));

	FFieldVariant Popped = Ctx.Properties.Pop();
	if (Property != Popped)
		return DC_FAIL(DcDSerDe, RecursiveSerializeTopPropertyChanged);

	return DcOk();
}

EDcSerializePredicateResult PredicateIsRootProperty(FDcSerializeContext& Ctx)
{
	check(Ctx.Properties.Num() > 0);
    return Ctx.TopProperty() == Ctx.Properties[0]
        ? EDcSerializePredicateResult::Process
        : EDcSerializePredicateResult::Pass;
}

} // namespace DcSerializeUtils

