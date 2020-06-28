#include "Deserialize/DcDeserializer.h"
#include "DcErrorCodes.h"

namespace DataConfig
{

FResult FDeserializer::Deserialize(FDeserializeContext& Ctx)
{
	check(Ctx.Deserializer == this);
	check(Ctx.Reader != nullptr);
	check(Ctx.Writer != nullptr);
	check(Ctx.Properties.Num() > 0);

	UField* Property = Ctx.TopProperty();
	FDeserializeDelegate* HandlerPtr = DirectDeserializersMap.Find(Property->GetClass());
	if (HandlerPtr == nullptr)
	{
		return Fail(EErrorCode::NoMatchingDeserializer);
	}

	EDeserializeResult HandlerRet;
	TRY(HandlerPtr->Execute(Ctx, HandlerRet));
	check(HandlerRet != EDeserializeResult::Unknown);

	if (HandlerRet == EDeserializeResult::CanNotProcess)
	{
		return Fail(EErrorCode::NoMatchingDeserializer);
	}
	else
	{
		return Ok();
	}
}

void FDeserializer::AddDirectHandler(UClass* PropertyClass, FDeserializeDelegate&& Delegate)
{
	check(!DirectDeserializersMap.Contains(PropertyClass));
	DirectDeserializersMap.Add(PropertyClass, MoveTemp(Delegate));
}

} // namespace DataConfig


