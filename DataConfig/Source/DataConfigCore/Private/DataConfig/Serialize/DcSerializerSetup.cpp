#include "DataConfig/Serialize/DcSerializerSetup.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Serialize/Handlers/Json/DcJsonCommonSerializers.h"
#include "DataConfig/Serialize/Handlers/Json/DcJsonObjectSerializers.h"
#include "DataConfig/Serialize/Handlers/Property/DcPropertyPipeSerializers.h"
#include "DataConfig/Serialize/Handlers/MsgPack/DcMsgPackCommonSerializers.h"
#include "DataConfig/Serialize/Handlers/MsgPack/DcMsgPackTransientSerializers.h"
#include "DataConfig/Serialize/Handlers/MsgPack/DcMsgPackPersistentSerializers.h"
#include "UObject/TextProperty.h"

void DcSetupJsonSerializeHandlers(FDcSerializer& Serializer, EDcJsonSerializeType Type)
{
	using namespace DcJsonHandlers;

	Serializer.AddDirectHandler(FBoolProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerBoolSerialize));
	Serializer.AddDirectHandler(FNameProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerNameSerialize));
	Serializer.AddDirectHandler(FStrProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStringSerialize));
	Serializer.AddDirectHandler(FTextProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTextSerialize));
	Serializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerFieldPathSerialize));

	{
		// order significant
		Serializer.AddPredicatedHandler(
			FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsEnumProperty),
			FDcSerializeDelegate::CreateStatic(HandlerEnumSerialize)
		);

		Serializer.AddPredicatedHandler(
			FDcSerializePredicate::CreateStatic(PredicateIsNumericProperty),
			FDcSerializeDelegate::CreateStatic(HandlerNumericSerialize)
		);
	}

	//	Containers
	Serializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerArraySerialize));
	Serializer.AddDirectHandler(FSetProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerSetSerialize));
	Serializer.AddDirectHandler(FMapProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerMapSerialize));

	//	Struct
	Serializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStructRootSerialize));
	Serializer.AddDirectHandler(FStructProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStructRootSerialize));

	//	Class
	Serializer.AddDirectHandler(UClass::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerClassRootSerialize));
	Serializer.AddPredicatedHandler(
		FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsSubObjectProperty),
		FDcSerializeDelegate::CreateStatic(HandlerInstancedSubObjectSerialize)
	);

	//	Object
	Serializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerObjectReferenceSerialize));
	Serializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerWeakObjectReferenceSerialize));
	Serializer.AddDirectHandler(FClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerClassReferenceSerialize));

	if (Type == EDcJsonSerializeType::Default)
	{
		Serializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerSoftObjectReferenceSerialize));
		Serializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerSoftClassReferenceSerialize));
		Serializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerLazyObjectReferenceSerialize));
	}
	else if (Type == EDcJsonSerializeType::StringSoftLazy)
	{
		Serializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStringSoftObjectSerialize));
		Serializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStringSoftClassSerialize));
		Serializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStringLazyObjectSerialize));
	}
	else
	{
		checkNoEntry();
	}
}

void DcSetupPropertyPipeSerializeHandlers(FDcSerializer& Serializer)
{
	using namespace DcPropertyPipeHandlers;

	Serializer.AddDirectHandler(UClass::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerClassSerialize));
	Serializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerClassSerialize));

	Serializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStructSerialize));
	Serializer.AddDirectHandler(FStructProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStructSerialize));

	Serializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerArraySerialize));
	Serializer.AddDirectHandler(FSetProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerSetSerialize));
	Serializer.AddDirectHandler(FMapProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerMapSerialize));

	DcPropertyUtils::VisitAllEffectivePropertyClass([&](FFieldClass* FieldClass) {
		if (!Serializer.FieldClassSerializerMap.Contains(FieldClass))
			Serializer.AddDirectHandler(FieldClass, FDcSerializeDelegate::CreateStatic(HandlerScalarSerialize));
	});
}

void DcSetupMsgPackSerializeHandlers(FDcSerializer& Serializer, EDcMsgPackSerializeType Type)
{
	using namespace DcMsgPackHandlers;

	Serializer.AddDirectHandler(UClass::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerClassSerialize));
	Serializer.AddDirectHandler(UScriptStruct::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStructSerialize));
	Serializer.AddDirectHandler(FStructProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerStructSerialize));

	Serializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerArraySerialize));
	Serializer.AddDirectHandler(FSetProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerSetSerialize));
	Serializer.AddDirectHandler(FMapProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerMapSerialize));

	Serializer.AddPredicatedHandler(
		FDcSerializePredicate::CreateStatic(&PredicateIsBlobProperty), 
		FDcSerializeDelegate::CreateStatic(&HandlerBlobSerialize)
	);

	if (Type == EDcMsgPackSerializeType::Default
		|| Type == EDcMsgPackSerializeType::StringSoftLazy)
	{
		Serializer.AddPredicatedHandler(
			FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsSubObjectProperty),
			FDcSerializeDelegate::CreateStatic(HandlerPersistentInstancedSubObjectSerialize)
		);

		Serializer.AddPredicatedHandler(
			FDcSerializePredicate::CreateStatic(DcSerializeUtils::PredicateIsEnumProperty),
			FDcSerializeDelegate::CreateStatic(HandlerPersistentEnumSerialize)
		);

		Serializer.AddDirectHandler(FNameProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentNameSerialize));
		Serializer.AddDirectHandler(FTextProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentTextSerialize));
		Serializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentFieldPathSerialize));
		Serializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentObjectReferenceSerialize));
		Serializer.AddDirectHandler(FClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentClassReferenceSerialize));
		Serializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentWeakObjectReferenceSerialize));

		if (Type == EDcMsgPackSerializeType::StringSoftLazy)
		{
			Serializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentSoftObjectReferenceSerialize));
			Serializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentSoftClassReferenceSerialize));
			Serializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentLazyObjectReferenceSerialize));
		}
		else
		{
			Serializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentStringSoftObjectSerialize));
			Serializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentStringSoftClassSerialize));
			Serializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPersistentStringLazyObjectSerialize));
		}
	}
	else if (Type == EDcMsgPackSerializeType::InMemory)
	{
		Serializer.AddDirectHandler(FNameProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientNameSerialize));
		Serializer.AddDirectHandler(FTextProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientTextSerialize));
		Serializer.AddDirectHandler(FObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientObjectSerialize));
		Serializer.AddDirectHandler(FClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientClassSerialize));
		Serializer.AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientSoftObjectSerialize));
		Serializer.AddDirectHandler(FSoftClassProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientSoftClassSerialize));
		Serializer.AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientWeakObjectSerialize));
		Serializer.AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientLazyObjectSerialize));
		Serializer.AddDirectHandler(FInterfaceProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientInterfaceSerialize));
		Serializer.AddDirectHandler(FDelegateProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientDelegateSerialize));
		Serializer.AddDirectHandler(FMulticastInlineDelegateProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientMulticastInlineDelegateSerialize));
		Serializer.AddDirectHandler(FMulticastSparseDelegateProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientMulticastSparseDelegateSerialize));
		Serializer.AddDirectHandler(FFieldPathProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientFieldPathSerialize));
		Serializer.AddDirectHandler(FEnumProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerTransientEnumSerialize));
	}
	else
	{
		checkNoEntry();
	}

	DcMsgPackUtils::VisitMsgPackPipeScalarPropertyClass([&](FFieldClass* FieldClass) {
		if (!Serializer.FieldClassSerializerMap.Contains(FieldClass))
			Serializer.AddDirectHandler(FieldClass, FDcSerializeDelegate::CreateStatic(HandlerScalarSerialize));
	});

}
