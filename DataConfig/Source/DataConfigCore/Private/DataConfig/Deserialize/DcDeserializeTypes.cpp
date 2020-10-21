#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DataConfig {

FDcResult FScopedProperty::PushProperty()
{
	DC_TRY(Ctx.Writer->PeekWriteProperty(&Property));
	Ctx.Properties.Push(Property);
	return DcOk();
}

FScopedProperty::~FScopedProperty()
{
	if (Property != nullptr)
	{
		UField* Popped = Ctx.Properties.Pop();
		check(Popped == Property);
	}
}

FScopedObject::FScopedObject(FDeserializeContext& InCtx, UObject* InObject)
	: Object(InObject)
	, Ctx(InCtx)
{
	check(Object);
	Ctx.Objects.Push(Object);
}

FScopedObject::~FScopedObject()
{
	UObject* Popped = Ctx.Objects.Pop();
	check(Popped == Object);
}

} // namespace DataConfig

