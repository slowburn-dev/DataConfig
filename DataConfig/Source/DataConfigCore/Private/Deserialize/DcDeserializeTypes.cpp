#include "Deserialize/DcDeserializeTypes.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig {

FResult FScopedProperty::PushProperty()
{
	TRY(Ctx.Writer->PeekWriteProperty(&Property));
	Ctx.Properties.Push(Property);
	return Ok();
}

FScopedProperty::~FScopedProperty()
{
	if (Property != nullptr)
	{
		UField* Popped = Ctx.Properties.Pop();
		check(Popped == Property);
	}
}

} // namespace DataConfig

