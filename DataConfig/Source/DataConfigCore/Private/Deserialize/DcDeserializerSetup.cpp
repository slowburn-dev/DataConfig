#include "Deserialize/DcDeserializerSetup.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "Deserialize/DcDeserializer.h"
#include "Deserialize/DcDeserializeTypes.h"
#include "Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "Deserialize/Handlers/DcStructDeserializers.h"

namespace DataConfig
{

void SetupDefaultDeserializeHandlers(FDeserializer& Deserializer)
{
	//	Primitives
	Deserializer.AddDirectHandler(UBoolProperty::StaticClass(), FDeserializeDelegate::CreateStatic(BoolDeserializeHandler));
	Deserializer.AddDirectHandler(UNameProperty::StaticClass(), FDeserializeDelegate::CreateStatic(NameDeserializeHandler));
	Deserializer.AddDirectHandler(UStrProperty::StaticClass(), FDeserializeDelegate::CreateStatic(StringDeserializeHandler));

	//	Struct
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDeserializeDelegate::CreateStatic(StructRootDeserializeHandler));
	Deserializer.AddDirectHandler(UStructProperty::StaticClass(), FDeserializeDelegate::CreateStatic(StructRootDeserializeHandler));
}

} // namespace DataConfig
