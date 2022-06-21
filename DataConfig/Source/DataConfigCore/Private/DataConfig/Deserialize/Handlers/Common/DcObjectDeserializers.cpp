#include "DataConfig/Deserialize/Handlers/Common/DcObjectDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"

namespace DcCommonHandlers {

FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerSoftObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeSoftObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerWeakObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeWeakObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerLazyObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeLazyObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeClassReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerSoftClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeSoftClassReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerStringToSoftObjectDeserialize(FDcDeserializeContext& Ctx)
{
	FString Value;
	DC_TRY(Ctx.Reader->ReadString(&Value));
	DC_TRY(Ctx.Writer->WriteSoftObjectReference(FSoftObjectPtr(FSoftObjectPath(Value))));
	return DcOk();
}

FDcResult HandlerStringToLazyObjectDeserialize(FDcDeserializeContext& Ctx)
{
	FString Value;
	DC_TRY(Ctx.Reader->ReadString(&Value));
	FGuid Guid;
	bool bOk = FGuid::Parse(Value, Guid);
	if (!bOk)
		return DC_FAIL(DcDSerDe, ParseGUIDFailed) << Value;

	FUniqueObjectGuid ObjGuid(Guid);
	FLazyObjectPtr LazyPtr;
	LazyPtr = ObjGuid;
	DC_TRY(Ctx.Writer->WriteLazyObjectReference(LazyPtr));
	return DcOk();
}

FDcResult HandlerStringToSoftClassDeserialize(FDcDeserializeContext& Ctx)
{
	FString Value;
	DC_TRY(Ctx.Reader->ReadString(&Value));
	DC_TRY(Ctx.Writer->WriteSoftClassReference(FSoftObjectPtr(FSoftClassPath(Value))));
	return DcOk();
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

FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeInstancedSubObject<TryReadTypeStr>(Ctx);
}

} // namespace DcCommonHandlers


