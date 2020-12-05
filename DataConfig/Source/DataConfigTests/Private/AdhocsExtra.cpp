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

void DeserializeExtra_AnyStructUsage()
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
			Any1.GetChecked<FDestructDelegateContainer>()->DestructAction.BindLambda([&DestructCalledCount] {
				DestructCalledCount++;
				});

			FDcAnyStruct Any2{ Any1 };
			FDcAnyStruct Any3 = MoveTemp(Any1);
			FDcAnyStruct Any4 = IdentityByValue(Any1);
		}

		check(DestructCalledCount == 1);
	}
}

void DeserializeExtra_AnyStruct()
{
	FDcDeserializer Deserializer;
	DcSetupDefaultDeserializeHandlers(Deserializer);

	//	any
	using namespace DcExtra;
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsDcAnyStruct),
		FDcDeserializeDelegate::CreateStatic(HandlerDcAnyStructDeserialize)
	);

	FStructExtraAny Struct;
	FDcPropertyWriter Writer(FDcPropertyDatum(FStructExtraAny::StaticStruct(), &Struct));

	FDcJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"Any1" : {
				"$type" : "TestStruct_Alpha",
				"AName" : "Foo",
				"ABool" : true,
				"AStr" : "these are my twisted words",
			},
			"Any2" : {
				"$type" : "FStructWithSet",
				"ASet" : [
					"a",
					"list",
					"of",
					"names",
				]
			}
		}
	)");
	Reader.SetNewString(*Str);

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FStructExtraAny::StaticStruct());
	Ctx.Prepare();
	Deserializer.Deserialize(Ctx);

	if (Struct.Any1.IsValid())
		Dump(FDcPropertyDatum(Struct.Any1.StructClass, &Struct.Any1));
	if (Struct.Any2.IsValid())
		Dump(FDcPropertyDatum(Struct.Any2.StructClass, &Struct.Any2));
}


