#include "Adhocs.h"
#include "Writer/Writer.h"
#include "Reader/PropertyReader.h"
#include "Writer/PropertyWriter.h"
#include "UObject/UnrealType.h"
#include "DataConfigTypes.h"
#include "DataConfigErrorCodes.h"

using namespace DataConfig;


void PropertyVisitorRoundtrip_ReadNested()
{
	FNestStruct1 NestStruct{};
	NestStruct.AName = FName(TEXT("Nest"));

	FTestStruct_Alpha& StructAlpha = NestStruct.AStruct;
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));

	FPropertyReader Reader(FPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));

	//	Root
	check(Reader.Peek() == EDataEntry::StructRoot);
	check(Reader.ReadStructRoot(nullptr, nullptr).Ok());

	//	AName
	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());
	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());

	//	Nest Struct
	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());
	check(Reader.Peek() == EDataEntry::StructRoot);
	check(Reader.ReadStructRoot(nullptr, nullptr).Ok());

	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());
	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());


	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());
	check(Reader.Peek() == EDataEntry::Bool);
	check(Reader.ReadBool(nullptr, nullptr).Ok());

	check(Reader.Peek() == EDataEntry::Name);
	check(Reader.ReadName(nullptr, nullptr).Ok());
	check(Reader.Peek() == EDataEntry::String);
	check(Reader.ReadString(nullptr, nullptr).Ok());

	check(Reader.Peek() == EDataEntry::StructEnd);
	check(Reader.ReadStructEnd(nullptr, nullptr).Ok());

	//	Pop 1 Level
	check(Reader.Peek() == EDataEntry::StructEnd);
	check(Reader.ReadStructEnd(nullptr, nullptr).Ok());

	check(Reader.Peek() == EDataEntry::Ended);
}


void PropertyVisitorRoundtrip()
{
	FTestStruct_Alpha StructAlpha{};
	//StructAlpha.AName = FName(TEXT("ALPHA"));
	//StructAlpha.ABool = true;
	//StructAlpha.AStr = FString(TEXT("A L P H A"));

	FPropertyWriter Writer(FPropertyDatum(FTestStruct_Alpha::StaticStruct(), &StructAlpha));

	check(Writer.Peek(EDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ALPHA"))).Ok());

	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ABool"))).Ok());
	check(Writer.Peek(EDataEntry::Bool).Ok());
	check(Writer.WriteBool(true).Ok());

	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStr"))).Ok());
	check(Writer.Peek(EDataEntry::String).Ok());
	check(Writer.WriteString(TEXT("A L P H A O N E")).Ok());

	check(Writer.Peek(EDataEntry::StructEnd).Ok());	// note that both End or Name are ok
	check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());
	check(Writer.Peek(EDataEntry::Ended).Ok());

	UE_LOG(LogDataConfigCore, Display, TEXT("bool: %d"), StructAlpha.ABool);
	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *StructAlpha.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("str: %s"), *StructAlpha.AStr);

	return;
}

