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

EDcSerializePredicateResult PredicateIsSubObjectProperty(FDcSerializeContext& Ctx)
{
	if (Ctx.TopProperty().IsUObject())
		return EDcSerializePredicateResult::Pass;

	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Ctx.TopProperty().ToFieldUnsafe());
	return ObjectProperty && DcPropertyUtils::IsSubObjectProperty(ObjectProperty)
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

EDcSerializePredicateResult PredicateIsEnumProperty(FDcSerializeContext& Ctx)
{
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;
	bool bFoundEnum = DcPropertyUtils::TryGetEnumPropertyOut(Ctx.TopProperty(), Enum, UnderlyingProperty);
	
	return bFoundEnum
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

} // namespace DcSerializeUtils

