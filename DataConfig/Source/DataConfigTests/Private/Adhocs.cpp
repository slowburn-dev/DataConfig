#include "Adhocs.h"
#include "Writer/DcWriter.h"
#include "Property/DcPropertyReader.h"
#include "Property/DcPropertyWriter.h"
#include "Writer/DcCompositeWriters.h"
#include "Writer/DcPrettyPrintWriter.h"
#include "UObject/UnrealType.h"
#include "DcTypes.h"
#include "DcErrorCodes.h"

using namespace DataConfig;

void PropertyVisitorRoundtrip_Piped()
{
	FTestStruct_ObjRef ObjRef;

	ObjRef.AlphaRef = NewObject<UTestObj_Alpha>();
	ObjRef.BetaRef = nullptr;
	ObjRef.AlphaRef->AStr = TEXT("Hello");
	ObjRef.AlphaRef->AStruct.AName = TEXT("InnerName");

	FTestStruct_ObjRef OutObj;
	OutObj.AlphaRef = NewObject<UTestObj_Alpha>();	// prepared here

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FPropertyReader Reader(FPropertyDatum(FTestStruct_ObjRef::StaticStruct(), &ObjRef));
		FPropertyWriter WriteWriter(FPropertyDatum(FTestStruct_ObjRef::StaticStruct(), &OutObj));

		FWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FPipeVisitor RoundtripVisit(&Reader, &Writer);
		FResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

void PropertyVisitorRoundtrip__TestObjAlpha()
{
	UTestObj_Alpha* Obj = NewObject<UTestObj_Alpha>();
	Obj->AStr = TEXT("A STR");
	Obj->AStruct.ABool = false;
	Obj->AStruct.AStr = "A Struct STr";
	Obj->AStruct.Names.Emplace(TEXT("One"));
	Obj->AStruct.Names.Emplace(TEXT("Two"));
	Obj->AStruct.Names.Emplace(TEXT("Three"));

	UTestObj_Alpha* OutObj = NewObject<UTestObj_Alpha>();

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FPropertyReader Reader(FPropertyDatum(UTestObj_Alpha::StaticClass(), Obj));
		FPropertyWriter WriteWriter(FPropertyDatum(UTestObj_Alpha::StaticClass(), OutObj));

		FWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FPipeVisitor RoundtripVisit(&Reader, &Writer);
		FResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}

}

void PropertyVisitorRoundtrip__StructStruct()
{
	FMapOfStruct2 St{};
	St.StructStructMap.Add(
		{TEXT("Key1"), true},
		{ TEXT("Value1"), }
	);

	St.StructStructMap.Add(
		{TEXT("Key2"), false},
		{ TEXT("Value2"), true, TEXT("Fuck") }
	);

	FMapOfStruct2 OutSt{};

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FPropertyReader Reader(FPropertyDatum(FMapOfStruct2::StaticStruct(), &St));
		FPropertyWriter WriteWriter(FPropertyDatum(FMapOfStruct2::StaticStruct(), &OutSt));

		FWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FPipeVisitor RoundtripVisit(&Reader, &Writer);
		FResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

void PropertyVisitorRoundtrip__StructHasMap()
{
	FMapOfStruct1 MapOfStruct{};
	MapOfStruct.StrStructMap.Add(TEXT("First"), { TEXT("What"), });
	MapOfStruct.StrStructMap.Add(TEXT("Second"), { TEXT("What"), true, TEXT("Fuck") });

	FMapOfStruct1 Out;

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FPropertyReader Reader(FPropertyDatum(FMapOfStruct1::StaticStruct(), &MapOfStruct));
		FPropertyWriter WriteWriter(FPropertyDatum(FMapOfStruct1::StaticStruct(), &Out));

		FWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FPipeVisitor RoundtripVisit(&Reader, &Writer);
		FResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

void PropertyVisitorRoundtrip__MapStruct()
{
	FMapContainer1 MapStruct{};
	MapStruct.StrStrMap.Add(TEXT("These"), TEXT("Are"));
	MapStruct.StrStrMap.Add(TEXT("My"), TEXT("Twisted"));
	MapStruct.StrStrMap.Add(TEXT("Words"), TEXT("."));

	MapStruct.NameBoolMap.Add(TEXT("true"), true);
	MapStruct.NameBoolMap.Add(TEXT("false"), false);

	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
	FPropertyReader Reader(FPropertyDatum(FMapContainer1::StaticStruct(), &MapStruct));
	FPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
	FPipeVisitor PrettyPrintVisit(&Reader, &Writer);
	FResult Ret = PrettyPrintVisit.PipeVisit();
	if (!Ret.Ok())
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
	}
}


void PropertyVisitorRoundtrip__Basic()
{
	FNestStruct1 NestStruct{};
	NestStruct.AName = FName(TEXT("Nest"));

	FTestStruct_Alpha& StructAlpha = NestStruct.AStruct;
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));


	FNestStruct1 OutStruct{};

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FPropertyReader Reader(FPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));
		FPropertyWriter WriteWriter(FPropertyDatum(FNestStruct1::StaticStruct(), &OutStruct));

		FWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FPipeVisitor RoundtripVisit(&Reader, &Writer);
		FResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

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


void PropertyVisitorRoundtrip_WriteNested()
{
	FNestStruct1 NestStruct{};
	FPropertyWriter Writer(FPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));

	//	Root
	check(Writer.Peek(EDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	//	Outer Name
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("OUTER"))).Ok());

	//	Nest Struct
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct"))).Ok());
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

	check(Writer.Peek(EDataEntry::StructEnd).Ok());	//	end nested
	check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.Peek(EDataEntry::StructEnd).Ok());	//	end outer struct
	check(Writer.WriteStructEnd(FNestStruct1::StaticStruct()->GetFName()).Ok());
	check(Writer.Peek(EDataEntry::Ended).Ok());

	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *NestStruct.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("bool: %d"), NestStruct.AStruct.ABool);
	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *NestStruct.AStruct.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("str: %s"), *NestStruct.AStruct.AStr);

	return;
}

void PropertyVisitorRoundtrip()
{
	FNestStruct1 NestStruct{};
	FPropertyWriter Writer(FPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));

	//	Root
	check(Writer.Peek(EDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FNestStruct1::StaticStruct()->GetFName()).Ok());

	//	Outer Name
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.SkipWrite().Ok());

	//	Nest Struct
	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct"))).Ok());
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
	check(Writer.SkipWrite().Ok());

	check(Writer.Peek(EDataEntry::StructEnd).Ok());	//	end nested
	check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.Peek(EDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct2"))).Ok());
	check(Writer.Peek(EDataEntry::StructRoot).Ok());
	check(Writer.SkipWrite().Ok());

	check(Writer.Peek(EDataEntry::StructEnd).Ok());	//	end outer struct
	check(Writer.WriteStructEnd(FNestStruct1::StaticStruct()->GetFName()).Ok());
	check(Writer.Peek(EDataEntry::Ended).Ok());

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FPropertyReader Reader(FPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));
		FPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
		FPipeVisitor PrettyPrintVisit(&Reader, &PrettyWriter);
		FResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}

	return;
}


