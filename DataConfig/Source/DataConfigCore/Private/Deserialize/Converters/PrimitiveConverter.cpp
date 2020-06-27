#include "Deserialize/Converters/PrimitiveConverter.h"

namespace DataConfig
{

bool FPrimitiveConverter::Prepare(FReader& Reader, FPropertyDatum& Datum, FDeserializeContext& Ctx)
{
	return Datum.IsA<UBoolProperty>()
		|| Datum.IsA<UNameProperty>()
		|| Datum.IsA<UStrProperty>();
}

FResult FPrimitiveConverter::Deserialize(FReader& Reader, FPropertyDatum& Datum, FDeserializeContext& Ctx)
{


	return Ok();
}

} // namespace DataConfig

