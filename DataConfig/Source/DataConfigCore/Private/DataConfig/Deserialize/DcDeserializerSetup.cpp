#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "DataConfig/Deserialize/Handlers/DcStructDeserializers.h"
#include "DataConfig/Deserialize/Handlers/DcClassDeserializers.h"

void DcSetupDefaultDeserializeHandlers(FDcDeserializer& Deserializer)
{
	//	Primitives
	Deserializer.AddDirectHandler(UBoolProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerBoolDeserialize));
	Deserializer.AddDirectHandler(UNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerNameDeserialize));
	Deserializer.AddDirectHandler(UStrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringDeserialize));

	//	Struct
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructRootDeserialize));
	Deserializer.AddDirectHandler(UStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructRootDeserialize));

	//	Class
	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassRootDeserialize));
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsSubObjectProperty),
		FDcDeserializeDelegate::CreateStatic(HandlerInstancedSubObjectDeserialize)
	);
	Deserializer.AddDirectHandler(UObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));

}

