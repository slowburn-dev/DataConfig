#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Property/DcPropertyWriter.h"

FDcResult FDcScopedProperty::PushProperty()
{
	DC_TRY(Ctx.Writer->PeekWriteProperty(&Property));
	Ctx.Properties.Push(Property);
	return DcOk();
}

FDcScopedProperty::~FDcScopedProperty()
{
	if (Property != nullptr)
	{
		UField* Popped = Ctx.Properties.Pop();
		check(Popped == Property);
	}
}

FDcScopedObject::FDcScopedObject(FDcDeserializeContext& InCtx, UObject* InObject)
	: Object(InObject)
	, Ctx(InCtx)
{
	check(Object);
	Ctx.Objects.Push(Object);
}

FDcScopedObject::~FDcScopedObject()
{
	UObject* Popped = Ctx.Objects.Pop();
	check(Popped == Object);
}


