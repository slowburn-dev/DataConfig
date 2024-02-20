#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Deserialize/Handlers/Common/DcObjectDeserializers.h"
#include "DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h"
#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackCommonDeserializers.h"
#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackTransientDeserializers.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"
#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
#include "UObject/PropertyOptional.h"
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

void DcSetupJsonDeserializeHandlers(FDcDeserializer& Deserializer, EDcJsonDeserializeType Type)
{
	using namespace DcCommonHandlers;

	{
		//	order significant
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsScalarArrayProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize)
		);

		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsEnumProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerStringToEnumDeserialize)
		);

		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsSubObjectProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerInstancedSubObjectDeserialize)
		);
	}

	AddNumericPipeDirectHandlers(Deserializer);

	//	Primitives
	Deserializer.AddDirectHandler(FBoolProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeBoolDeserialize));
	Deserializer.AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeNameDeserialize));
	Deserializer.AddDirectHandler(FStrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeStringDeserialize));
	Deserializer.AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeTextDeserialize));
	Deserializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToFieldPathDeserialize));

	//	Containers
	Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
	Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayToSetDeserialize));
	Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapOrArrayOfKeyValueDeserialize));

	//	Struct
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToStructDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToStructDeserialize));

	//	Class
	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToClassDeserialize));
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsSubObjectProperty),
		FDcDeserializeDelegate::CreateStatic(HandlerInstancedSubObjectDeserialize)
	);

	//	Object
	Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));
	Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassReferenceDeserialize));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
	Deserializer.AddDirectHandler(FObjectPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));
	Deserializer.AddDirectHandler(FClassPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassReferenceDeserialize));
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
	Deserializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerWeakObjectReferenceDeserialize));

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	Deserializer.AddDirectHandler(FOptionalProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerOptionalDeserialize));
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

	if (Type == EDcJsonDeserializeType::Default)
	{
		Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSoftObjectReferenceDeserialize));
		Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSoftClassReferenceDeserialize));
		Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerLazyObjectReferenceDeserialize));
	}
	else if (Type == EDcJsonDeserializeType::StringSoftLazy)
	{
		Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToSoftObjectDeserialize));
		Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToSoftClassDeserialize));
		Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToLazyObjectDeserialize));
	}
	else
	{
		checkNoEntry();
	}
}

void DcSetupPropertyPipeDeserializeHandlers(FDcDeserializer& Deserializer)
{
	{
		//	order significant
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(DcCommonHandlers::PredicateIsScalarArrayProperty),
			FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerArrayDeserialize)
		);
	}

	using namespace DcPropertyPipeHandlers;

	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassDeserialize));

	Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassDeserialize));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
	Deserializer.AddDirectHandler(FObjectPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassDeserialize));
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3

	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructDeserialize));

	Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerArrayDeserialize));
	Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSetDeserialize));
	Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapDeserialize));

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	Deserializer.AddDirectHandler(FOptionalProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerOptionalDeserialize));
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

	DcPropertyUtils::VisitAllEffectivePropertyClass([&](FFieldClass* FieldClass) {
		if (!Deserializer.FieldClassDeserializerMap.Contains(FieldClass))
			Deserializer.AddDirectHandler(FieldClass, FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerPipeScalarDeserialize));
	});
}

void DcSetupMsgPackDeserializeHandlers(FDcDeserializer& Deserializer, EDcMsgPackDeserializeType Type)
{
	using namespace DcCommonHandlers;
	{
		//	order significant
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsScalarArrayProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize)
		);
	}

	AddNumericPipeDirectHandlers(Deserializer);
	
	Deserializer.AddDirectHandler(FBoolProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeBoolDeserialize));
	Deserializer.AddDirectHandler(FStrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeStringDeserialize));

	//	Containers
	Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
	Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayToSetDeserialize));
	Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcMsgPackHandlers::HandlerMapDeserialize));
	
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	Deserializer.AddDirectHandler(FOptionalProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerOptionalDeserialize));
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

	//	Struct
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToStructDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToStructDeserialize));

	//	Class
	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToClassDeserialize));

	//	Blob
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(DcMsgPackHandlers::PredicateIsBlobProperty), 
		FDcDeserializeDelegate::CreateStatic(DcMsgPackHandlers::HandlerBlobDeserialize)
	);

	if (Type == EDcMsgPackDeserializeType::Default
		|| Type == EDcMsgPackDeserializeType::StringSoftLazy)
	{
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsSubObjectProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerInstancedSubObjectDeserialize)
		);

		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsEnumProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerStringToEnumDeserialize)
		);

		Deserializer.AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeNameDeserialize));
		Deserializer.AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeTextDeserialize));
		Deserializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToFieldPathDeserialize));
		Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));
		Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassReferenceDeserialize));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
		Deserializer.AddDirectHandler(FObjectPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));
		Deserializer.AddDirectHandler(FClassPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassReferenceDeserialize));
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3

		Deserializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerWeakObjectReferenceDeserialize));

		if (Type == EDcMsgPackDeserializeType::StringSoftLazy)
		{
			Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToSoftObjectDeserialize));
			Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToSoftClassDeserialize));
			Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringToLazyObjectDeserialize));
		}
		else
		{
			Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSoftObjectReferenceDeserialize));
			Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSoftClassReferenceDeserialize));
			Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerLazyObjectReferenceDeserialize));
		}
	}
	else if (Type == EDcMsgPackDeserializeType::InMemory)
	{
		using namespace DcMsgPackHandlers;
		Deserializer.AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientNameDeserialize));
		Deserializer.AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientTextDeserialize));
		Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientObjectDeserialize));
		Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientClassDeserialize));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
		Deserializer.AddDirectHandler(FObjectPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientObjectDeserialize));
		Deserializer.AddDirectHandler(FClassPtrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientClassDeserialize));
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
		Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientSoftObjectDeserialize));
		Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientSoftClassDeserialize));
		Deserializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientWeakObjectDeserialize));
		Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientLazyObjectDeserialize));
		Deserializer.AddDirectHandler(FInterfaceProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientInterfaceDeserialize));
		Deserializer.AddDirectHandler(FDelegateProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientDelegateDeserialize));
		Deserializer.AddDirectHandler(FMulticastInlineDelegateProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientMulticastInlineDelegateDeserialize));
		Deserializer.AddDirectHandler(FMulticastSparseDelegateProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientMulticastSparseDelegateDeserialize));
		Deserializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientFieldPathDeserialize));
		Deserializer.AddDirectHandler(FEnumProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientEnumDeserialize));
	}
	else
	{
		checkNoEntry();
	}
}

