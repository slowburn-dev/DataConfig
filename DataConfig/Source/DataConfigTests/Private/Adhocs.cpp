#include "Adhocs.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Writer/DcCompositeWriters.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "UObject/UnrealType.h"
#include "DataConfig/DcTypes.h"

#include "DataConfig/Misc/DcDataVariant.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"

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
		FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FDcPropertyReader Reader(FDcPropertyDatum(FTestStruct_ObjRef::StaticStruct(), &ObjRef));
		FDcPropertyWriter WriteWriter(FDcPropertyDatum(FTestStruct_ObjRef::StaticStruct(), &OutObj));

		FDcWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
		FDcResult Ret = RoundtripVisit.PipeVisit();
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
		FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FDcPropertyReader Reader(FDcPropertyDatum(UTestObj_Alpha::StaticClass(), Obj));
		FDcPropertyWriter WriteWriter(FDcPropertyDatum(UTestObj_Alpha::StaticClass(), OutObj));

		FDcWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
		FDcResult Ret = RoundtripVisit.PipeVisit();
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
		FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FDcPropertyReader Reader(FDcPropertyDatum(FMapOfStruct2::StaticStruct(), &St));
		FDcPropertyWriter WriteWriter(FDcPropertyDatum(FMapOfStruct2::StaticStruct(), &OutSt));

		FDcWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
		FDcResult Ret = RoundtripVisit.PipeVisit();
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
		FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FDcPropertyReader Reader(FDcPropertyDatum(FMapOfStruct1::StaticStruct(), &MapOfStruct));
		FDcPropertyWriter WriteWriter(FDcPropertyDatum(FMapOfStruct1::StaticStruct(), &Out));

		FDcWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
		FDcResult Ret = RoundtripVisit.PipeVisit();
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
	FDcPropertyReader Reader(FDcPropertyDatum(FMapContainer1::StaticStruct(), &MapStruct));
	FDcPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
	FDcPipeVisitor PrettyPrintVisit(&Reader, &Writer);
	FDcResult Ret = PrettyPrintVisit.PipeVisit();
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
		FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FDcPropertyReader Reader(FDcPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));
		FDcPropertyWriter WriteWriter(FDcPropertyDatum(FNestStruct1::StaticStruct(), &OutStruct));

		FDcWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
		FDcResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

static EDcDataEntry _ReaderPeek(FDcReader& Reader)
{
	EDcDataEntry Cur;
	check(Reader.PeekRead(&Cur).Ok());
	return Cur;
}

void PropertyVisitorRoundtrip_ReadNested()
{
	FNestStruct_OldSchool NestStruct{};
	NestStruct.AName = FName(TEXT("Nest"));

	FTestStruct_OldSchool& StructAlpha = NestStruct.AStruct;
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));

	FDcPropertyReader Reader(FDcPropertyDatum(FNestStruct_OldSchool::StaticStruct(), &NestStruct));

	//	Root
	check(_ReaderPeek(Reader) == EDcDataEntry::StructRoot);
	check(Reader.ReadStructRoot(nullptr).Ok());

	//	AName
	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());
	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());

	//	Nest Struct
	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());
	check(_ReaderPeek(Reader) == EDcDataEntry::StructRoot);
	check(Reader.ReadStructRoot(nullptr).Ok());

	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());
	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());


	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());
	check(_ReaderPeek(Reader) == EDcDataEntry::Bool);
	check(Reader.ReadBool(nullptr).Ok());

	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	check(Reader.ReadName(nullptr).Ok());
	check(_ReaderPeek(Reader) == EDcDataEntry::String);
	check(Reader.ReadString(nullptr).Ok());

	check(_ReaderPeek(Reader) == EDcDataEntry::StructEnd);
	check(Reader.ReadStructEnd(nullptr).Ok());

	//	Pop 1 Level
	check(_ReaderPeek(Reader) == EDcDataEntry::StructEnd);
	check(Reader.ReadStructEnd(nullptr).Ok());

	check(_ReaderPeek(Reader) == EDcDataEntry::Ended);
}


void PropertyVisitorRoundtrip_WriteNested()
{
	FNestStruct1 NestStruct{};
	FDcPropertyWriter Writer(FDcPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));

	//	Root
	check(Writer.PeekWrite(EDcDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FNestStruct1::StaticStruct()->GetFName()).Ok());

	//	Outer Name
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("OUTER"))).Ok());

	//	Nest Struct
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ALPHA"))).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ABool"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Bool).Ok());
	check(Writer.WriteBool(true).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStr"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::String).Ok());
	check(Writer.WriteString(TEXT("A L P H A O N E")).Ok());

	check(Writer.PeekWrite(EDcDataEntry::StructEnd).Ok());	//	end nested
	check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.PeekWrite(EDcDataEntry::StructEnd).Ok());	//	end outer struct
	check(Writer.WriteStructEnd(FNestStruct1::StaticStruct()->GetFName()).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Ended).Ok());

	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *NestStruct.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("bool: %d"), NestStruct.AStruct.ABool);
	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *NestStruct.AStruct.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("str: %s"), *NestStruct.AStruct.AStr);

	return;
}

void PropertyVisitorRoundtrip()
{
	FNestStruct1 NestStruct{};
	FDcPropertyWriter Writer(FDcPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));

	//	Root
	check(Writer.PeekWrite(EDcDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FNestStruct1::StaticStruct()->GetFName()).Ok());

	//	Outer Name
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.SkipWrite().Ok());

	//	Nest Struct
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ALPHA"))).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ABool"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Bool).Ok());
	check(Writer.WriteBool(true).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStr"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::String).Ok());
	check(Writer.SkipWrite().Ok());

	check(Writer.PeekWrite(EDcDataEntry::StructEnd).Ok());	//	end nested
	check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.PeekWrite(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct2"))).Ok());
	check(Writer.PeekWrite(EDcDataEntry::StructRoot).Ok());
	check(Writer.SkipWrite().Ok());

	check(Writer.PeekWrite(EDcDataEntry::StructEnd).Ok());	//	end outer struct
	check(Writer.WriteStructEnd(FNestStruct1::StaticStruct()->GetFName()).Ok());
	check(Writer.PeekWrite(EDcDataEntry::Ended).Ok());

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FDcPropertyReader Reader(FDcPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));
		FDcPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
		FDcPipeVisitor PrettyPrintVisit(&Reader, &PrettyWriter);
		FDcResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}

	return;
}

void TryOutVariant()
{
	{
		FDcDataVariant DV = false;
		check(DV.DataType == EDcDataEntry::Bool);
	}

	{
		FDcDataVariant DV = FName(TEXT("Wha"));
		check(DV.DataType == EDcDataEntry::Name);
		check(DV.GetValue<FName>() == FName(("Wha")));
	}

	{
		FDcDataVariant DV = TEXT("Str");
		check(DV.DataType == EDcDataEntry::String);
		check(DV.GetValue<FString>() == FString(("Str")));
	}

}

void TryOutPutback()
{
	FEmptyStruct EmptyObj;

	FDcPropertyReader RawReader(FDcPropertyDatum(FEmptyStruct::StaticStruct(), &EmptyObj));
	FDcPutbackReader Reader(&RawReader);

	check(_ReaderPeek(Reader) == EDcDataEntry::StructRoot);
	check(Reader.ReadStructRoot(nullptr).Ok());

	Reader.Putback(FName(TEXT("Foo")));
	Reader.Putback(FName(TEXT("Bar")));

	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	FName Name1;
	check(Reader.ReadName(&Name1).Ok());
	check(Name1 == FName(TEXT("Foo")));

	check(_ReaderPeek(Reader) == EDcDataEntry::Name);
	FName Name2;
	check(Reader.ReadName(&Name2).Ok());
	check(Name2 == FName(TEXT("Bar")));

	check(_ReaderPeek(Reader) == EDcDataEntry::StructEnd);
	check(Reader.ReadStructEnd(nullptr).Ok());

	check(_ReaderPeek(Reader) == EDcDataEntry::Ended);
}

void TryDiags()
{
	DcPushEnv();

	DcEnv().Diag({0, 0}) << TEXT("WTF");
	DcEnv().Diag({1, 1}) << true;

	//	if this works i think it's pretty ok now
	auto What = []() -> FDcResult {
		return DcEnv().Diag({1, 2}) << TEXT("Fuck My Life");
	};

	check(!What().Ok());
}

void TryConsoleDiagnosticReports()
{
	FDcWriter Writer{};
	Writer.WriteBool(true);

	DcEnv().Diag(DC_DIAG(DcDCommon, Unreachable));

	TCHAR Ch = TCHAR('t');
	FString Str(1, &Ch);

	DcEnv().Diag(DC_DIAG(DcDCommon, Unexpected1)) << Str;
	DcEnv().Diag(DC_DIAG(DcDCommon, Unexpected1)) << EDcDataEntry::Bool;
}



