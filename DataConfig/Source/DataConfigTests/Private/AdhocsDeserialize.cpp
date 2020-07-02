#include "Adhocs.h"
#include "Deserialize/DcDeserializer.h"
#include "Deserialize/DcDeserializeTypes.h"
#include "Deserialize/DcDeserializerSetup.h"
#include "Json/DcJsonReader.h"
#include "Property/DcPropertyWriter.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"

static void Dump(FPropertyDatum Datum)
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
	FPropertyReader PropReader(Datum);
	FPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
	FPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);
	FResult Ret = PrettyPrintVisit.PipeVisit();
	if (!Ret.Ok())
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
	}
}


void DeserializeSimple()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"AName" : "FromJson",
			"ABool" : true,
			"AStr" : "Wooooooot",
		}
	)");
	Reader.SetNewString(&Str);

	FTestStruct_Alpha OutAlpha;
	FPropertyWriter Writer(FPropertyDatum(FTestStruct_Alpha::StaticStruct(), &OutAlpha));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FTestStruct_Alpha::StaticStruct());
	Deserializer.Deserialize(Ctx);

	Dump(FPropertyDatum(FTestStruct_Alpha::StaticStruct(), &OutAlpha));
}


void DeserializeNestedStruct()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"AName" : "FromJson",
			"AStruct" : {
				"ABool" : true,
				"AStr" : "Nest1",
			},
			"AStruct2" : {
				"ABool" : false,
				"AStr" : "Nest2",
			},
		}
	)");
	Reader.SetNewString(&Str);

	FNestStruct1 OutNest;
	FPropertyWriter Writer(FPropertyDatum(FNestStruct1::StaticStruct(), &OutNest));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FNestStruct1::StaticStruct());
	Deserializer.Deserialize(Ctx);


	Dump(FPropertyDatum(FNestStruct1::StaticStruct(), &OutNest));
}

void DeserializeObjectRoot()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	UTestClass_Alpha* Obj = NewObject<UTestClass_Alpha>();
	FPropertyWriter Writer(FPropertyDatum(UTestClass_Alpha::StaticClass(), Obj));

	FJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"AName" : "FromJson",
			"ABool" : true,
			"AStr" : "Wooooooot",
			"AStruct" : {
				"ABool" : true,
				"AStr" : "Nest1",
			},
		}
	)");
	Reader.SetNewString(&Str);

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(UTestClass_Alpha::StaticClass());
	Deserializer.Deserialize(Ctx);

	Dump(FPropertyDatum(UTestClass_Alpha::StaticClass(), Obj));
}

void DeserializeObjectRef()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FObjReference ObjRef{};		// default initialized OR READER WILL THROW
	FPropertyWriter Writer(FPropertyDatum(FObjReference::StaticStruct(), &ObjRef));

	FJsonReader Reader;
	//	TODO
	//	for now we can only get this to work, figure a way to add access raw asset or add something
	//	in DataConfigTests/Content

	//	TODO get a fixture uasset working with Program target, shoudn't be that hard
	//		"Obj2" : "/Fixture/DataLayer",
	FString Str = TEXT(R"(
		{
			"Obj1" : "Object'/Script/DataConfigTests'",
		}
	)");
	Reader.SetNewString(&Str);

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FObjReference::StaticStruct());
	Deserializer.Deserialize(Ctx);

	Dump(FPropertyDatum(FObjReference::StaticStruct(), &ObjRef));
}

void WriteFixtureAsset()
{
	//	TODO still not working due to some weird assertions
	//	pass for now, and we'll only need to create the package somehow

	//	ref: `UAssetToolsImpl::CreateAsset`
	FString OutPath(TEXT("/Fixture/SimpleObject"));
	UPackage* Package = CreatePackage(nullptr, *OutPath);
	Package->FullyLoad();
	EObjectFlags Flags = RF_Public | RF_Standalone | RF_Transactional;
	//	note that it will check Abstractness, so use the one with this binary
	UObject* NewObj = NewObject<UObject>(Package, UEmptyObject::StaticClass(), TEXT("SimpleObject"), Flags);

	//	ref: UNGUnrealEditorStatics::ImportFBXAsCurveContainer
	Package->MarkPackageDirty();
	Package->GetMetaData(); // touch metadata to create it
	FString PackageFileName = FPackageName::LongPackageNameToFilename(OutPath, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, NewObj, RF_Public | RF_Standalone, *PackageFileName);
	check(bSaved);
}



