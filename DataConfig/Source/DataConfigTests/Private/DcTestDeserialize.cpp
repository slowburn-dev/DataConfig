#include "DcTestDeserialize.h"
#include "DcTestProperty.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

static FDcResult _DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum)
{
	FDcDeserializer Deserializer;
	DcSetupJsonDeserializeHandlers(Deserializer);

	FDcPropertyWriter Writer(Datum);
	FDcDeserializeContext Ctx;
	Ctx.Reader = Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(Datum.Property);
	Ctx.Prepare();

	return Deserializer.Deserialize(Ctx);
}


DC_TEST("DataConfig.Core.Deserializer.Primitive1")
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"BoolField" : true,
			"NameField" : "AName",
			"StringField" : "AStr",
			"TextField" : "AText",
			"EnumField" : "Tard",

			"FloatField" : 12.3,
			"DoubleField" : 23.4,

			"Int8Field" : -43,
			"Int16Field" : -2243,
			"Int32Field" : -23415,
			"Int64Field" : -1524523,

			"UInt8Field" : 213,
			"UInt16Field" : 2243,
			"UInt32Field" : 23415,
			"UInt64Field" : 1524523,
		}

	)");
	Reader.SetNewString(*Str);

	FDcTestStruct1 Dest;
	FDcPropertyDatum Datum(FDcTestStruct1::StaticStruct(), &Dest);

	UTEST_OK("Deserialize into FDcTestStruct1", _DeserializeJsonInto(&Reader, Datum));


	return true;
}



