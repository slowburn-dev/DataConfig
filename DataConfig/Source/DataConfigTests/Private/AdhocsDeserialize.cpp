#include "Adhocs.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"

static void Dump(FDcPropertyDatum Datum)
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
	FDcPropertyReader PropReader(Datum);
	FDcPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
	FDcPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);
	FDcResult Ret = PrettyPrintVisit.PipeVisit();
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

	FDcJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"AName" : "FromJson",
			"ABool" : true,
			"AStr" : "Wooooooot",
		}
	)");
	Reader.SetNewString(&Str);

	FTestStruct_Alpha OutAlpha;
	FDcPropertyWriter Writer(FDcPropertyDatum(FTestStruct_Alpha::StaticStruct(), &OutAlpha));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FTestStruct_Alpha::StaticStruct());
	Deserializer.Deserialize(Ctx);

	Dump(FDcPropertyDatum(FTestStruct_Alpha::StaticStruct(), &OutAlpha));
}


void DeserializeNestedStruct()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FDcJsonReader Reader;
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
	FDcPropertyWriter Writer(FDcPropertyDatum(FNestStruct1::StaticStruct(), &OutNest));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(FNestStruct1::StaticStruct());
	Deserializer.Deserialize(Ctx);


	Dump(FDcPropertyDatum(FNestStruct1::StaticStruct(), &OutNest));
}

void DeserializeObjectRoot()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	UTestClass_Alpha* Obj = NewObject<UTestClass_Alpha>();
	FDcPropertyWriter Writer(FDcPropertyDatum(UTestClass_Alpha::StaticClass(), Obj));

	FDcJsonReader Reader;
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
	Ctx.Objects.Push(Obj);
	Deserializer.Deserialize(Ctx);

	Dump(FDcPropertyDatum(UTestClass_Alpha::StaticClass(), Obj));
}

void DeserializeObjectRef()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FObjReference ObjRef{};		// default initialized OR READER WILL THROW
	FDcPropertyWriter Writer(FDcPropertyDatum(FObjReference::StaticStruct(), &ObjRef));

	FDcJsonReader Reader;
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

	Dump(FDcPropertyDatum(FObjReference::StaticStruct(), &ObjRef));
}

void DeserializeSubObject()
{
	using namespace DataConfig;

	FDeserializer Deserializer;
	SetupDefaultDeserializeHandlers(Deserializer);

	FShapeContainer Obj{};
	FDcPropertyWriter Writer(FDcPropertyDatum(FShapeContainer::StaticStruct(), &Obj));

	FDcJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"ShapeAlpha" :  {
				"$type" : "ShapeBox",
				"ShapeName" : "Box1"
			},
			"ShapeBeta" : {
				"$type" : "ShapeSquare",
				"ShapeName" : "Square1"
			},
			"ShapeGamma" : {
				"ShapeName" : "BaseShape"
			}
		}
	)");
	Reader.SetNewString(&Str);

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Objects.Push(GetTransientPackage());	// need this as base
	Ctx.Properties.Push(FShapeContainer::StaticStruct());
	Deserializer.Deserialize(Ctx);

	Dump(FDcPropertyDatum(FShapeContainer::StaticStruct(), &Obj));
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



