#include "Old/Adhocs.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeBase64.h"

void DeserializeExtra_Color() {
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
	//	and color
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
		FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
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
			"StrInBetween" : "woooooot",
			"Any2" : {
				"$type" : "StructExtraColor",
				"Pre" : "<Pre>",
				"Post" : "<Post>",
				"Blue" : "#0000FF00",
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

	check(Struct.Any1.SharedReferenceCount.GetSharedReferenceCount() == 1);
	check(Struct.Any2.SharedReferenceCount.GetSharedReferenceCount() == 1);

	Dump(FDcPropertyDatum(FStructExtraAny::StaticStruct(), &Struct));
	if (Struct.Any1.IsValid())
		Dump(FDcPropertyDatum(Struct.Any1.StructClass, Struct.Any1.DataPtr));
	if (Struct.Any2.IsValid())
		Dump(FDcPropertyDatum(Struct.Any2.StructClass, Struct.Any2.DataPtr));
}

void DeserializeExtra_Base64()
{
	FDcDeserializer Deserializer;
	DcSetupDefaultDeserializeHandlers(Deserializer);

	//	base64
	using namespace DcExtra;
	Deserializer.AddPredicatedHandler(
		FDcDeserializePredicate::CreateStatic(PredicateIsBase64Blob),
		FDcDeserializeDelegate::CreateStatic(HandleBase64BlobDeserialize)
	);

	FStructExtraBlob Struct;
	FDcPropertyWriter Writer(FDcPropertyDatum(FStructExtraBlob::StaticStruct(), &Struct));

	FDcJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"Pre" : "Woot",
			"Blob" : "dGhlc2UgYXJlIG15IHR3aXN0ZWQgd29yZHM=",
			"Post" : [1,2,3],
		}
	)");
	Reader.SetNewString(*Str);

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FStructExtraBlob::StaticStruct());
	Ctx.Prepare();
	Deserializer.Deserialize(Ctx);

	FString Base64Str(Struct.Blob.Num(), (ANSICHAR*)Struct.Blob.GetData());
	check(TEXT("these are my twisted words" == Base64Str));

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		UE_LOG(
			LogDataConfigCore,
			Display,
			TEXT("delegate name: %s"),
			*FDcDeserializePredicate::CreateStatic(PredicateIsBase64Blob).TryGetBoundFunctionName().ToString()
		);
	}

	Dump(FDcPropertyDatum(FStructExtraBlob::StaticStruct(), &Struct));
}


