#include "Deserialize/DcDeserializer.h"
#include "DcErrorCodes.h"

namespace DataConfig
{

FResult FDeserializer::Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx)
{
	check(Ctx.Deserializer == this);

	for (TSharedRef<IDeserializeConverter>& Converter : Converters)
	{
		if (Converter->Prepare(Reader, Writer, Ctx))
		{
			return Converter->Deserialize(Reader, Writer, Ctx);
		}
	}

	return Fail(EErrorCode::NoMatchingDeserializer);
}

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

	if (HandlerRet == EDeserializeResult::CanNotProcess)
	{
		return Fail(EErrorCode::NoMatchingDeserializer);
	}
	else
	{
		return Ok();
	}
}

void FDeserializer::AddConverter(const TSharedRef<IDeserializeConverter>& NewConverter)
{
	Converters.Add(NewConverter);
}

void FDeserializer::AddDirectHandler(UClass* PropertyClass, FDeserializeDelegate&& Delegate)
{
	check(!DirectDeserializersMap.Contains(PropertyClass));
	DirectDeserializersMap.Add(PropertyClass, MoveTemp(Delegate));
}

} // namespace DataConfig


