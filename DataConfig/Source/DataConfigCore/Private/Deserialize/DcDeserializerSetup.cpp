#include "Deserialize/DcDeserializerSetup.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "Deserialize/DcDeserializer.h"
#include "Deserialize/DcDeserializeTypes.h"
#include "Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "Deserialize/Handlers/DcStructDeserializers.h"
#include "Deserialize/Handlers/DcClassDeserializers.h"

namespace DataConfig
{

void SetupDefaultDeserializeHandlers(FDeserializer& Deserializer)
{
	//	Primitives
	Deserializer.AddDirectHandler(UBoolProperty::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerBoolDeserialize));
	Deserializer.AddDirectHandler(UNameProperty::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerNameDeserialize));
	Deserializer.AddDirectHandler(UStrProperty::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerStringDeserialize));

	//	Struct
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerStructRootDeserialize));
	Deserializer.AddDirectHandler(UStructProperty::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerStructRootDeserialize));

	//	Class
	Deserializer.AddDirectHandler(UClass::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerClassRootDeserialize));
	Deserializer.AddPredicatedHandler(
		FDeserializePredicate::CreateStatic(PredicateIsSubObjectProperty), 
		FDeserializeDelegate::CreateStatic(HandlerInstancedSubObjectDeserialize)
	);
	Deserializer.AddDirectHandler(UObjectProperty::StaticClass(), FDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));

}

} // namespace DataConfig
