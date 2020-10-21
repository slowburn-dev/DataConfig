#include "DataConfigImportFactory.h"
#include "ImportedInterface.h"
#include "EditorFramework/AssetImportData.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "DataConfigImportFactory"

//	TODO note that this breaks unity build, get rid of it or do it as namespace FOO_Private
namespace
{

using namespace DataConfig;

static FResult ReadRootTypeFromMapping(FReader& Reader, UClass*& OutDataClass)
{
	TRY(Reader.ReadMapRoot(nullptr));

	FString Value;
	TRY(Reader.ReadString(&Value, nullptr));

	if (Value != TEXT("$type"))
		return Fail();

	TRY(Reader.ReadString(&Value, nullptr));

	//	TODO support path and other things, for now just use FindObject<UClass>
	OutDataClass = FindObject<UClass>(ANY_PACKAGE, *Value, true);
	return OutDataClass ? Ok() : Fail();
}

static TOptional<DataConfig::FDeserializer> DcDeserializer;
static int32 LoadJSONAssetCount;

template<typename T>
struct TScopedCheckSingleUse : private FNoncopyable
{
	using TVolatile = volatile T;

	TScopedCheckSingleUse(T& InCounter)
		: Counter(InCounter)
	{
		T Count = FPlatformAtomics::InterlockedIncrement(((TVolatile*)&Counter));
		check(Count == 1);
	}

	~TScopedCheckSingleUse()
	{
		T Count = FPlatformAtomics::InterlockedDecrement(((TVolatile*)&Counter));
		check(Count == 0);
	}

	T& Counter;
};

static void LazyInitializeDeserializer()
{
	if (!DcDeserializer.IsSet())
	{
		DcDeserializer.Emplace();
		SetupDefaultDeserializeHandlers(DcDeserializer.GetValue());
	}
}

static FResult TryLoadJSONAsset(FString &JSONStr, UClass* DataClass, UObject* NewObj)
{
	TScopedCheckSingleUse<int32> LockDeserailizer(LoadJSONAssetCount);

	LazyInitializeDeserializer();

	FJsonReader Reader(&JSONStr);
	FPropertyWriter Writer(FPropertyDatum(DataClass, NewObj));

	FDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &DcDeserializer.GetValue();
	Ctx.Properties.Push(DataClass);

	return DcDeserializer->Deserialize(Ctx);
}

}	// end of namespace

UDataConfigImportFactory::UDataConfigImportFactory()
{
	//	ref: UCSVImportFactory
	bCreateNew = false;
	bEditAfterNew = true;
	//	this is a base pass and it's dynamically resolved at `DoesSupportClass`
	SupportedClass = UDataAsset::StaticClass();	

	bEditorImport = true;
	bText = false;	// this only determines it's char* or TCHAR* read in

	Formats.Add(TEXT("json;JSON files"));
}

FText UDataConfigImportFactory::GetDisplayName() const
{
	return LOCTEXT("DataConfigImportFactoryDescription", "DataConfig Importer");
}


UObject* UDataConfigImportFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	//	ref: CSVImportFactory
	//FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	//	ref: FbxFactory
	UObject* ExistingObject = StaticFindObject(UObject::StaticClass(), InParent, *(InName.ToString()));
	if (ExistingObject)
	{
		//	it's a reimport
		EReimportResult::Type Ret = Reimport(ExistingObject);
		if (Ret == EReimportResult::Succeeded)
		{
			return ExistingObject;
		}
		else
		{
			//	TODO should fallback or prompt to create new asset? 
			return nullptr;
		}
	}

	FString JSONStr((int32)(BufferEnd - Buffer), (char*)Buffer);
	UClass* DataClass = nullptr;

	using namespace DataConfig;

	{
		FJsonReader TypeReader(&JSONStr);
		FResult Ret = ReadRootTypeFromMapping(TypeReader, DataClass);
		if (!Ret.Ok())
		{
			return nullptr;
		}
	}

	if (!DoesSupportClass(DataClass))
	{
		return nullptr;
	}

	UDataAsset* NewObj = NewObject<UDataAsset>(InParent, DataClass, InName, Flags);
	FResult Ret = TryLoadJSONAsset(JSONStr, DataClass, NewObj);
	if (!Ret.Ok())
	{
		//	TODO proper destroy the object if import failed
		NewObj->ConditionalBeginDestroy();
		DataConfig::Env().FlushDiags();
		return nullptr;
	}

	//	finalizing
	IImportedInterface* Imported = CastChecked<IImportedInterface>(NewObj);
	check(Imported);
	Imported->SetAssetImportData(NewObject<UAssetImportData>(NewObj, TEXT("AssetImportData")));
	Imported->GetAssetImportData()->Update(CurrentFilename);

	//FEditorDelegates::OnAssetPostImport.Broadcast(this, NewObj);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, NewObj);

	return NewObj;
}

bool UDataConfigImportFactory::DoesSupportClass(UClass* Class)
{
	return Class->IsChildOf<UDataAsset>()
		&& Class->ImplementsInterface(UImportedInterface::StaticClass());
}

bool UDataConfigImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension.Compare(TEXT("json"), ESearchCase::IgnoreCase) == 0;
}

bool UDataConfigImportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	//	ref: UReimportCurveTableFactory::CanReimport
	IImportedInterface* Imported = Cast<IImportedInterface>(Obj);
	if (Imported
		&& Imported->GetAssetImportData())
	{
		Imported->GetAssetImportData()->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UDataConfigImportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	IImportedInterface* Imported = Cast<IImportedInterface>(Obj);
	if (Imported
		&& ensure(NewReimportPaths.Num() == 1))
	{
		Imported->GetAssetImportData()->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UDataConfigImportFactory::Reimport(UObject* Obj)
{
	IImportedInterface* Imported = Cast<IImportedInterface>(Obj);
	if (!Imported)
	{
		return EReimportResult::Failed;
	}

	if (!Imported->GetAssetImportData())
	{
		return EReimportResult::Failed;
	}

	FString SourceFileName = Imported->GetAssetImportData()->GetFirstFilename();
	TArray<uint8> FileBuf;
	if (!FFileHelper::LoadFileToArray(FileBuf, *SourceFileName))
	{
		return EReimportResult::Failed;
	}


	FString JSONStr((int32)(FileBuf.Num()), (char*)(FileBuf.GetData()));
	UClass* DataClass = nullptr;

	{
		FJsonReader TypeReader(&JSONStr);
		FResult Ret = ReadRootTypeFromMapping(TypeReader, DataClass);
		if (!Ret.Ok())
		{
			return EReimportResult::Failed;
		}
	}

	if (DataClass != Obj->GetClass())
	{
		//	class mismatch
		return EReimportResult::Failed;
	}

	//	reimport into existing object
	FResult Ret = TryLoadJSONAsset(JSONStr, DataClass, Obj);
	if (!Ret.Ok())
	{
		DataConfig::Env().FlushDiags();
		return EReimportResult::Failed;
	}

	return EReimportResult::Succeeded;
}

int32 UDataConfigImportFactory::GetPriority() const
{
	return ImportPriority;
}


#undef LOCTEXT_NAMESPACE

