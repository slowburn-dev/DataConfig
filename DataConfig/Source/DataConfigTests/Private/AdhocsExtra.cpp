#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.h"

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


static FDcAnyStruct IdentityByValue(FDcAnyStruct Handle)
{
	return Handle;
}

void DeserializeExtra_Usage()
{
	{
		FDcAnyStruct Alhpa(new FTestStruct_Alpha());
		FDcAnyStruct Beta = Alhpa;

		check(Alhpa.GetSharedReferenceCount() == 2);
	}

	{
		uint32 DestructCalledCount = 0;
		{
			FDcAnyStruct Any1(new FDestructDelegateContainer());
			Any1.GetChecked<FDestructDelegateContainer>()->DestructAction.BindLambda([&] {
				DestructCalledCount++;
				});

			FDcAnyStruct Any2{ Any1 };
			FDcAnyStruct Any3 = MoveTemp(Any1);
			FDcAnyStruct Any4 = IdentityByValue(Any1);
		}

		check(DestructCalledCount == 1);
	}
}


