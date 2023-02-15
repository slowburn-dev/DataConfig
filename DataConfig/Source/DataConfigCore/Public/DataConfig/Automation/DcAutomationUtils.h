#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializerSetup.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
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

///	Access `gDcDebug` in MSVC immediate window:
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

enum class EReadDatumEqualType
{
	Default,			//	default
	ExpandAllObjects	//	expand all objects.
						//	Note that this might cause infinite loop on cyclic references
};

DATACONFIGCORE_API FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum, EReadDatumEqualType Type = EReadDatumEqualType::Default);

DATACONFIGCORE_API FDcResult DumpNextNumericAsString(FDcReader* Reader, FString* OutStr);

DATACONFIGCORE_API void DumpToLog(FDcPropertyDatum Datum);
DATACONFIGCORE_API void DumpToLowLevelDebugOutput(FDcPropertyDatum Datum);
DATACONFIGCORE_API FString DumpFormat(FDcPropertyDatum Datum);

DATACONFIGCORE_API void DumpToLog(FDcReader* Reader);
DATACONFIGCORE_API void DumpToLowLevelDebugOutput(FDcReader* Reader);
DATACONFIGCORE_API FString DumpFormat(FDcReader* Reader);

#if WITH_EDITORONLY_DATA
DATACONFIGCORE_API void AmendMetaData(UField* Field, const FName& MetaKey, const TCHAR* MetaValue);
DATACONFIGCORE_API void AmendMetaData(UStruct* Struct, const FName& FieldName, const FName& MetaKey, const TCHAR* MetaValue);
#endif // WITH_EDITORONLY_DATA

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

	return Property->GetPropertyValue(Property->template ContainerPtrToValuePtr<T>(Datum.DataPtr));
}

DATACONFIGCORE_API int DebugGetEnumPropertyIndex(const FDcPropertyDatum& Datum, const FName& Name);
DATACONFIGCORE_API double DebugGetRealPropertyValue(const FDcPropertyDatum& Datum, const FName& Name);

enum class EDefaultSetupType
{
	SetupJSONHandlers,
	SetupNothing,
};


DATACONFIGCORE_API FDcResult DeserializeFrom(FDcReader* Reader, FDcPropertyDatum Datum, TFunctionRef<void(FDcDeserializeContext&)> Func, EDefaultSetupType SetupType = EDefaultSetupType::SetupJSONHandlers);
FORCEINLINE FDcResult DeserializeFrom(FDcReader* Reader, FDcPropertyDatum Datum)
{
	return DeserializeFrom(Reader, Datum, [](FDcDeserializeContext&)
	{
		/*pass*/
	});
}

DATACONFIGCORE_API FDcResult SerializeInto(FDcWriter* Writer, FDcPropertyDatum Datum, TFunctionRef<void(FDcSerializeContext&)> Func, EDefaultSetupType SetupType = EDefaultSetupType::SetupJSONHandlers);
FORCEINLINE FDcResult SerializeInto(FDcWriter* Writer, FDcPropertyDatum Datum)
{
	return SerializeInto(Writer, Datum, [](FDcSerializeContext&)
	{
		/*pass*/
	});
}

DATACONFIGCORE_API FString DcReindentStringLiteral(FString Str, FString* Prefix = nullptr);

}	// namespace DcAutomationUtils



