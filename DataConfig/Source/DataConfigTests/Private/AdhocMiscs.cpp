#include "Adhocs.h"
#include "StructSerializer.h"
#include "StructDeserializer.h"
#include "Backends/JsonStructSerializerBackend.h"
#include "Serialization/JsonWriter.h"

#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

#include "CoreMinimal.h"

void DumpStruct(void* StructPtr, UStruct* StructClass)
{
	TArray<uint8> Buffer;
	FMemoryWriter Writer(Buffer);

	FJsonStructSerializerBackend Backend(Writer, EStructSerializerBackendFlags::Default);
	FStructSerializerPolicies Policies;

	FStructSerializer::Serialize(
		StructPtr,
		*StructClass,
		Backend,
		Policies
	);

	//	believe or not it's just writes to TCHAR
	FString Ret(Buffer.Num() / 2, (TCHAR*)Buffer.GetData());
	UE_LOG(LogDataConfigCore, Display, L"Write To:\n %s", *Ret);
}


void UEStructSerializer()
{
	{
		FTestStruct_Alpha StructAlpha;
		StructAlpha.AName = L"What";
		StructAlpha.ABool = true;
		StructAlpha.AStr = L"Stringy";
		StructAlpha.Names = {
			L"These",
			L"Are",
		};

		DumpStruct(&StructAlpha, FTestStruct_Alpha::StaticStruct());
	}

	{
		FTestStruct_ObjRef StructObjRef;
		StructObjRef.AlphaRef = NewObject<UTestObj_Alpha>();
		StructObjRef.BetaRef = nullptr;

		DumpStruct(&StructObjRef, FTestStruct_ObjRef::StaticStruct());
	}

	TArray<uint8> Value;
	FMemoryWriter Reader(Value, true);
	int Val = 123;
	Reader << Val;
	/*
	//	MemoryArchive hard coded doesn't support this one
	UObject* Obj = nullptr;
	Reader << Obj;
	UClass* Cls = UTestObj_Alpha::StaticClass();
	Reader << Cls;
	*/
}






