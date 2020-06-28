#include "Deserialize/DcDeserializeTypes.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig {


IDeserializeConverter::~IDeserializeConverter()
{}

FResult FScopedProperty::PushProperty()
{
	TRY(Ctx.Writer->GetWriteProperty(&Property));
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

