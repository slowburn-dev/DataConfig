#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyDatum.h"

namespace DcAutomationUtils
{

DATACONFIGCORE_API FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum);

DATACONFIGCORE_API FDcResult DumpToLog(FDcPropertyDatum Datum);

DATACONFIGCORE_API void AmendMetaData(UField* Field, const FName& MetaKey, const TCHAR* MetaValue);
DATACONFIGCORE_API void AmendMetaData(UStruct* Struct, const FName& FieldName, const FName& MetaKey, const TCHAR* MetaValue);

template<typename TThunk>
FDcResult DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum, const TThunk& Func)
{
	FDcDeserializer Deserializer;
	DcSetupJsonDeserializeHandlers(Deserializer);

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



