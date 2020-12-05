#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"

void DeserializeExtra_Color()
{
	FDcDeserializer Deserializer;
	DcSetupDefaultDeserializeHandlers(Deserializer);

	//	colored
	using namespace DcExtra;
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
		FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
	);

	FStructExtraColor Struct;
	FDcPropertyWriter Writer(FDcPropertyDatum(FStructExtraColor::StaticStruct(), &Struct));

	FDcJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"Pre" : "<Pre>",
			"Post" : "<Post>",
			"Blue" : "#0000FF00",
		}
	)");
	Reader.SetNewString(*Str);

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FStructExtraColor::StaticStruct());
	Ctx.Prepare();
	Deserializer.Deserialize(Ctx);

	check(Struct.Blue.B == 0xFF);

	Dump(FDcPropertyDatum(FStructExtraColor::StaticStruct(), &Struct));

}



