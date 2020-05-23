#include "Adhocs.h"
#include "Writer/Writer.h"
#include "Reader/PropertyReader.h"
#include "Writer/PropertyWriter.h"
#include "UObject/UnrealType.h"
#include "DataConfigTypes.h"
#include "DataConfigErrorCodes.h"

using namespace DataConfig;


void PropertyVisitorRoundtrip()
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


void PropertyVisitorRoundtrip_Basics()
{
	FTestStruct_Alpha StructAlpha{};
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));

	FPropertyReader Reader(FPropertyDatum(FTestStruct_Alpha::StaticStruct(), &StructAlpha));

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
	check(Reader.Peek() == EDataEntry::Ended);

	return;
}

