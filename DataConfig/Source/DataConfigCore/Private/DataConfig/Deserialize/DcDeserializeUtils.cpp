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

EDcDeserializePredicateResult PredicateIsEnumProperty(FDcDeserializeContext& Ctx)
{
	UEnum* Enum;
	FNumericProperty* UnderlyingProperty;
	bool bFoundEnum = DcPropertyUtils::TryGetEnumPropertyOut(Ctx.TopProperty(), Enum, UnderlyingProperty);
	
	return bFoundEnum
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx)
{
	if (Ctx.TopProperty().IsUObject())
		return EDcDeserializePredicateResult::Pass;

	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Ctx.TopProperty().ToFieldUnsafe());
	return ObjectProperty && DcPropertyUtils::IsSubObjectProperty(ObjectProperty)
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

} // namespace DcDeserializeUtils



