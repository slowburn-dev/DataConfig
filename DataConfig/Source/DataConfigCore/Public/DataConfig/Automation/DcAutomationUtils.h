#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyUtils.h"

#include "UObject/PropertyAccessUtil.h"

#if DC_BUILD_DEBUG

struct DATACONFIGCORE_API FDcDebug
{
	FORCENOINLINE void DumpStruct(char* StructNameChars, void* Ptr);
	FORCENOINLINE void DumpObject(UObject* Obj);
	FORCENOINLINE void DumpDatum(void* DatumPtr);
	FORCENOINLINE FString DumpName(FName* NamePtr);
};

///	Access `gDcDebugg` in MSVC immediate window:
///
///	- in monolith builds:
///	gDcDebug.DumpObject(Obj)
///
///	- in DLL builds prefix with dll name:
///	({,,UE4Editor-DataConfigCore}gDcDebug).DumpObject(ObjPtr)

extern FDcDebug gDcDebug;
#endif // DC_BUILD_DEBUG

namespace DcAutomationUtils
{

DATACONFIGCORE_API FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum);

DATACONFIGCORE_API void DumpToLog(FDcPropertyDatum Datum);
DATACONFIGCORE_API void DumpToLowLevelDebugOutput(FDcPropertyDatum Datum);
DATACONFIGCORE_API FString DumpFormat(FDcPropertyDatum Datum);


DATACONFIGCORE_API void AmendMetaData(UField* Field, const FName& MetaKey, const TCHAR* MetaValue);
DATACONFIGCORE_API void AmendMetaData(UStruct* Struct, const FName& FieldName, const FName& MetaKey, const TCHAR* MetaValue);

DATACONFIGCORE_API FDcPropertyDatum TryGetMemberDatum(const FDcPropertyDatum& Datum, const FName& Name);

template<typename T>
T DebugGetScalarPropertyValue(const FDcPropertyDatum& Datum, const FName& Name)
{
	using TProperty = typename DcPropertyUtils::TPropertyTypeMap<T>::Type;

	UStruct* Struct = DcPropertyUtils::TryGetStruct(Datum);
	if (!Struct)
		return T{};

	TProperty* Property = CastField<TProperty>(PropertyAccessUtil::FindPropertyByName(Name, Struct));
	if (!Property)
		return T{};

	return Property->GetPropertyValue(Property->ContainerPtrToValuePtr<T>(Datum.DataPtr));
}

enum class EDefaultSetupType
{
	SetupJSONHandlers,
	SetupNothing,
};

template<typename TThunk>
FDcResult DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum, const TThunk& Func, EDefaultSetupType SetupType = EDefaultSetupType::SetupJSONHandlers)
{
	FDcDeserializer Deserializer;
	if (SetupType == EDefaultSetupType::SetupJSONHandlers)
	{
		DcSetupJsonDeserializeHandlers(Deserializer);
	}
	else if (SetupType == EDefaultSetupType::SetupNothing)
	{
		//	pass
	}

	FDcPropertyWriter Writer(Datum);
	FDcDeserializeContext Ctx;
	Ctx.Reader = Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(Datum.Property);

	Func((FDcDeserializer&)Deserializer, (FDcDeserializeContext&)Ctx);
	DC_TRY(Ctx.Prepare());

	return Deserializer.Deserialize(Ctx);
}

FORCEINLINE FDcResult DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum)
{
	return DeserializeJsonInto(Reader, Datum, [](FDcDeserializer&, FDcDeserializeContext&)
	{
		/*pass*/
	});
}

}	// namespace DcAutomationUtils



