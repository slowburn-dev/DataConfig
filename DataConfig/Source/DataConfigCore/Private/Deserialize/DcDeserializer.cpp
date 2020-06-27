#include "Deserialize/DcDeserializer.h"

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

void FDeserializer::AddConverter(const TSharedRef<IDeserializeConverter>& NewConverter)
{
	Converters.Add(NewConverter);
}

} // namespace DataConfig


