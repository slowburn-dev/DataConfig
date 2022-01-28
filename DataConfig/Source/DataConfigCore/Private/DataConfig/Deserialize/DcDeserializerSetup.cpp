#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/Handlers/Json/DcJsonCommonDeserializers.h"
#include "DataConfig/Deserialize/Handlers/Json/DcJsonObjectDeserializers.h"
#include "DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h"
#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackCommonDeserializers.h"
#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackTransientDeserializers.h"
#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackPersistentDeserializers.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"

void DcSetupJsonDeserializeHandlers(FDcDeserializer& Deserializer, EDcJsonDeserializeType Type)
{
	using namespace DcJsonHandlers;

	//	Primitives
	Deserializer.AddDirectHandler(FBoolProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerBoolDeserialize));
	Deserializer.AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerNameDeserialize));
	Deserializer.AddDirectHandler(FStrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringDeserialize));
	Deserializer.AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTextDeserialize));
	Deserializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerFieldPathDeserialize));

	{
		// order significant
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsEnumProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerEnumDeserialize)
		);

		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsNumericProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerNumericDeserialize)
		);
	}

	//	Containers
	Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
	Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSetDeserialize));
	Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapDeserialize));

	//	Struct
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructRootDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructRootDeserialize));

	//	Class
	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassRootDeserialize));
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsSubObjectProperty),
		FDcDeserializeDelegate::CreateStatic(HandlerInstancedSubObjectDeserialize)
	);

	//	Object
	Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerObjectReferenceDeserialize));
	Deserializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerWeakObjectReferenceDeserialize));
	Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassReferenceDeserialize));

	if (Type == EDcJsonDeserializeType::Default)
	{
		Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSoftObjectReferenceDeserialize));
		Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSoftClassReferenceDeserialize));
		Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerLazyObjectReferenceDeserialize));
	}
	else if (Type == EDcJsonDeserializeType::StringSoftLazy)
	{
		Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringSoftObjectDeserialize));
		Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringSoftClassDeserialize));
		Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStringLazyObjectDeserialize));
	}
	else
	{
		checkNoEntry();
	}
}

void DATACONFIGCORE_API DcSetupPropertyPipeDeserializeHandlers(FDcDeserializer& Deserializer)
{
	using namespace DcPropertyPipeHandlers;

	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassDeserialize));
	Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassDeserialize));

	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructDeserialize));

	Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
	Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSetDeserialize));
	Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapDeserialize));

	DcPropertyUtils::VisitAllEffectivePropertyClass([&](FFieldClass* FieldClass) {
		if (!Deserializer.FieldClassDeserializerMap.Contains(FieldClass))
			Deserializer.AddDirectHandler(FieldClass, FDcDeserializeDelegate::CreateStatic(HandlerScalarDeserialize));
	});
}

void DcSetupMsgPackDeserializeHandlers(FDcDeserializer& Deserializer, EDcMsgPackDeserializeType Type)
{
	using namespace DcMsgPackHandlers;

	Deserializer.AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerClassDeserialize));
	Deserializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructDeserialize));
	Deserializer.AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerStructDeserialize));

	Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
	Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSetDeserialize));
	Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapDeserialize));

	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(&PredicateIsBlobProperty), 
		FDcDeserializeDelegate::CreateStatic(&HandlerBlobDeserialize)
	);

	if (Type == EDcMsgPackDeserializeType::Default
		|| Type == EDcMsgPackDeserializeType::StringSoftLazy)
	{
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsSubObjectProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerPersistentInstancedSubObjectDeserialize)
		);

		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsEnumProperty),
			FDcDeserializeDelegate::CreateStatic(HandlerPersistentEnumDeserialize)
		);

		Deserializer.AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentNameDeserialize));
		Deserializer.AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentTextDeserialize));
		Deserializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentFieldPathDeserialize));
		Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentObjectReferenceDeserialize));
		Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentClassReferenceDeserialize));
		Deserializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentWeakObjectReferenceDeserialize));

		if (Type == EDcMsgPackDeserializeType::StringSoftLazy)
		{
			Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentSoftObjectReferenceDeserialize));
			Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentSoftClassReferenceDeserialize));
			Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentLazyObjectReferenceDeserialize));
		}
		else
		{
			Deserializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentStringSoftObjectDeserialize));
			Deserializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentStringSoftClassDeserialize));
			Deserializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPersistentStringLazyObjectDeserialize));
		}
	}
	else if (Type == EDcMsgPackDeserializeType::InMemory)
	{
		Deserializer.AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientNameDeserialize));
		Deserializer.AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientTextDeserialize));
		Deserializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientObjectDeserialize));
		Deserializer.AddDirectHandler(FClassProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerTransientClassDeserialize));
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

	DcMsgPackUtils::VisitMsgPackPipeScalarPropertyClass([&](FFieldClass* FieldClass) {
		if (!Deserializer.FieldClassDeserializerMap.Contains(FieldClass))
			Deserializer.AddDirectHandler(FieldClass, FDcDeserializeDelegate::CreateStatic(HandlerScalarDeserialize));
	});
}

