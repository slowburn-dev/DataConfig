#include "DataConfig/Serialize/Handlers/Common/DcObjectSerializers.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerializeCommon.inl"
#include "DataConfig/Serialize/DcSerializeUtils.h"

namespace DcCommonHandlers {

FDcResult HandlerObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerSoftObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerWeakObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeWeakObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerLazyObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeLazyObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeClassReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerSoftClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftClassReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerSoftObjectToStringSerialize(FDcSerializeContext& Ctx)
{
	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftObjectReference(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	return DcOk();
}

FDcResult HandlerLazyObjectToStringSerialize(FDcSerializeContext& Ctx)
{
	FLazyObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadLazyObjectReference(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.GetUniqueID().ToString()));
	return DcOk();
}

FDcResult HandlerSoftClassToStringSerialize(FDcSerializeContext& Ctx)
{
	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftClassReference(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
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

FDcResult HandlerInstancedSubObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeInstancedSubObject<&TryWriteTypeStr>(Ctx);
}


} // namespace DcCommonHandlers





