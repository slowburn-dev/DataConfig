#include "Adhocs.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Writer/DcWeakCompositeWriter.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "UObject/UnrealType.h"
#include "DataConfig/DcTypes.h"

#include "DataConfig/Misc/DcDataVariant.h"
#include "DataConfig/Reader/DcPutbackReader.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Writer/DcNoopWriter.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

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

		RoundtripVisit.PostVisit.BindLambda([&](FDcPipeVisitor* Self) {
			FString LineRead = ((FDcPropertyReader*)(Self->Reader))->FormatHighlight().Formatted;
			UE_LOG(LogDataConfigCore, Display, TEXT("- Read: %s"), *LineRead);
			FString LineWrite = WriteWriter.FormatHighlight().Formatted;
			UE_LOG(LogDataConfigCore, Display, TEXT("- Write: %s"), *LineWrite);
			});

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

	PrettyPrintVisit.PostVisit.BindLambda([](FDcPipeVisitor* Self) {
		FString Line = ((FDcPropertyReader*)(Self->Reader))->FormatHighlight().Formatted;
		UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Line);
	});


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
		RoundtripVisit.PostVisit.BindLambda([&](FDcPipeVisitor* Self) {
			FString Line = ((FDcPropertyReader*)(Self->Reader))->FormatHighlight().Formatted;
			UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Line);
			FString LineWrite = WriteWriter.FormatHighlight().Formatted;
			UE_LOG(LogDataConfigCore, Display, TEXT("- Write: %s"), *LineWrite);
		});

		FDcResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

static EDcDataEntry _ReaderPeek(FDcReader& Reader)
{
	EDcDataEntry Cur;
	check(Reader.ReadNext(&Cur).Ok());
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
	check(Writer.WriteNext(EDcDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FNestStruct1::StaticStruct()->GetFName()).Ok());

	//	Outer Name
	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("OUTER"))).Ok());

	//	Nest Struct
	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStruct"))).Ok());
	check(Writer.WriteNext(EDcDataEntry::StructRoot).Ok());
	check(Writer.WriteStructRoot(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AName"))).Ok());
	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ALPHA"))).Ok());

	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("ABool"))).Ok());
	check(Writer.WriteNext(EDcDataEntry::Bool).Ok());
	check(Writer.WriteBool(true).Ok());

	check(Writer.WriteNext(EDcDataEntry::Name).Ok());
	check(Writer.WriteName(FName(TEXT("AStr"))).Ok());
	check(Writer.WriteNext(EDcDataEntry::String).Ok());
	check(Writer.WriteString(TEXT("A L P H A O N E")).Ok());

	check(Writer.WriteNext(EDcDataEntry::StructEnd).Ok());	//	end nested
	check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

	check(Writer.WriteNext(EDcDataEntry::StructEnd).Ok());	//	end outer struct
	check(Writer.WriteStructEnd(FNestStruct1::StaticStruct()->GetFName()).Ok());
	check(Writer.WriteNext(EDcDataEntry::Ended).Ok());

	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *NestStruct.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("bool: %d"), NestStruct.AStruct.ABool);
	UE_LOG(LogDataConfigCore, Display, TEXT("name: %s"), *NestStruct.AStruct.AName.ToString());
	UE_LOG(LogDataConfigCore, Display, TEXT("str: %s"), *NestStruct.AStruct.AStr);

	return;
}

void PropertyVisitorRoundtrip()
{
	FNestStruct1 NestStruct{};
	{
		FDcPropertyWriter Writer(FDcPropertyDatum(FNestStruct1::StaticStruct(), &NestStruct));

		//	Root
		check(Writer.WriteNext(EDcDataEntry::StructRoot).Ok());
		check(Writer.WriteStructRoot(FNestStruct1::StaticStruct()->GetFName()).Ok());

		//	Outer Name
		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("AName"))).Ok());
		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.SkipWrite().Ok());

		//	Nest Struct
		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("AStruct"))).Ok());
		check(Writer.WriteNext(EDcDataEntry::StructRoot).Ok());
		check(Writer.WriteStructRoot(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("AName"))).Ok());
		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("ALPHA"))).Ok());

		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("ABool"))).Ok());
		check(Writer.WriteNext(EDcDataEntry::Bool).Ok());
		check(Writer.WriteBool(true).Ok());

		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("AStr"))).Ok());
		check(Writer.WriteNext(EDcDataEntry::String).Ok());
		check(Writer.SkipWrite().Ok());

		check(Writer.WriteNext(EDcDataEntry::StructEnd).Ok());	//	end nested
		check(Writer.WriteStructEnd(FTestStruct_Alpha::StaticStruct()->GetFName()).Ok());

		check(Writer.WriteNext(EDcDataEntry::Name).Ok());
		check(Writer.WriteName(FName(TEXT("AStruct2"))).Ok());
		check(Writer.WriteNext(EDcDataEntry::StructRoot).Ok());
		check(Writer.SkipWrite().Ok());

		check(Writer.WriteNext(EDcDataEntry::StructEnd).Ok());	//	end outer struct
		check(Writer.WriteStructEnd(FNestStruct1::StaticStruct()->GetFName()).Ok());
		check(Writer.WriteNext(EDcDataEntry::Ended).Ok());
	}

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


void TryTemplates()
{
	/*
	DcPushEnv();

	{
		FDcNoopWriter NoopWriter; FDcScopedActiveWriter ScopeWriter(&NoopWriter);

		{
			FDcNoopWriter InnerWriter;
			TDcStoreThenReset<FDcWriter*> NestReader(DcEnv().ActiveWriter, &InnerWriter);

			check(DcEnv().ActiveWriter == &InnerWriter);
		}

		check(DcEnv().ActiveWriter == &NoopWriter);
	}

	check(DcEnv().ActiveWriter == nullptr);
	*/
}

void PropertyVisitorRoundtrip__Set()
{
	FStructWithSet SetStruct{};

	SetStruct.ASet.Add(FName(TEXT("End")));
	SetStruct.ASet.Add(FName(TEXT("It")));
	SetStruct.ASet.Add(FName(TEXT("Now")));

	FStructWithSet OutStruct{};

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

		FDcPropertyReader Reader(FDcPropertyDatum(FStructWithSet::StaticStruct(), &SetStruct));
		FDcPropertyWriter WriteWriter(FDcPropertyDatum(FStructWithSet::StaticStruct(), &OutStruct));

		FDcWeakCompositeWriter Writer;
		Writer.Writers.Add(&PrettyPrinter);
		Writer.Writers.Add(&WriteWriter);

		FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
		RoundtripVisit.PostVisit.BindLambda([&](FDcPipeVisitor* Self) {
			FString Line = ((FDcPropertyReader*)(Self->Reader))->FormatHighlight().Formatted;
			UE_LOG(LogDataConfigCore, Display, TEXT("%s"), *Line);
			FString LineWrite = WriteWriter.FormatHighlight().Formatted;
			UE_LOG(LogDataConfigCore, Display, TEXT("- Write: %s"), *LineWrite);
			});

		FDcResult Ret = RoundtripVisit.PipeVisit();
		if (!Ret.Ok())
			UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
	}
}

static void _Roundtrip(FDcPropertyDatum ReadDatum, FDcPropertyDatum WriteDatum)
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
	FDcPrettyPrintWriter PrettyPrinter(*(FOutputDevice*)GWarn);

	FDcPropertyReader Reader(ReadDatum);
	FDcPropertyWriter WriteWriter(WriteDatum);

	FDcWeakCompositeWriter Writer;
	Writer.Writers.Add(&PrettyPrinter);
	Writer.Writers.Add(&WriteWriter);

	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	RoundtripVisit.PostVisit.BindLambda([&](FDcPipeVisitor* Self) {
		FString Line = ((FDcPropertyReader*)(Self->Reader))->FormatHighlight().Formatted;
		UE_LOG(LogDataConfigCore, Display, TEXT("- Read  : %s"), *Line);
		FString LineWrite = WriteWriter.FormatHighlight().Formatted;
		UE_LOG(LogDataConfigCore, Display, TEXT("- Write : %s"), *LineWrite);
		});

	FDcResult Ret = RoundtripVisit.PipeVisit();
	if (!Ret.Ok())
		UE_LOG(LogDataConfigCore, Display, TEXT("- roundtrip visit failed --"));
}

void PropertyVisitorRoundtrip__Enum()
{
	FStructWithEnum Struct;

	Struct.Enum1 = EFootbar::Bart;
	Struct.Enum2 = EIntBased::NegOne;
	Struct.Enum3 = EBPFlags::Alpha | EBPFlags::Beta | EBPFlags::Gamma;

	FStructWithEnum OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithEnum::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithEnum::StaticStruct(), &OutStruct)
	);
}


void PropertyVisitorRoundtrip__Text()
{
	FStructWithText Struct;

	Struct.Text1 = FText(NSLOCTEXT("DcAdhocs", "T1", "These are my twised words"));

	FStructWithText OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithText::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithText::StaticStruct(), &OutStruct)
	);
}


void PropertyVisitorRoundtrip__ClassReference()
{
	FStructWithClassReference Struct;

	Struct.Cls1 = UTestObj_Alpha::StaticClass();
	Struct.Cls2 = nullptr;
	Struct.Cls3 = UShapeBox::StaticClass();

	FStructWithClassReference OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithClassReference::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithClassReference::StaticStruct(), &OutStruct)
	);
}


void PropertyVisitorRoundtrip__ScriptStruct()
{
	FStructWithScriptStruct Struct;

	Struct.Struct1 = FNestStruct1::StaticStruct();

	FStructWithScriptStruct OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithScriptStruct::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithScriptStruct::StaticStruct(), &OutStruct)
	);
}

void PropertyVisitorRoundtrip__Objects()
{
	FTestStruct_ObjRef Struct;

	Struct.AlphaRef = NewObject<UTestObj_Alpha>();
	Struct.BetaRef = nullptr;

	FTestStruct_ObjRef OutStruct{};
	OutStruct.AlphaRef = NewObject<UTestObj_Alpha>();
	OutStruct.BetaRef = nullptr;

	_Roundtrip(
		FDcPropertyDatum(FTestStruct_ObjRef::StaticStruct(), &Struct),
		FDcPropertyDatum(FTestStruct_ObjRef::StaticStruct(), &OutStruct)
	);
}

void PropertyVisitorRoundtrip_SoftWeakLazy()
{
	FStructWithSoftObjectPtr Struct;

	Struct.Weak1 = UTestObj_Alpha::StaticClass();
	Struct.Lazy1 = UTestObj_Alpha::StaticClass();
	Struct.Soft1 = UTestObj_Alpha::StaticClass();
	Struct.Soft2 = UTestObj_Alpha::StaticClass();

	/*
	{
		FDcReader Reader;
		Reader.ReadSoftClassPtr(&Struct.Soft2);
	}
	*/

	FStructWithSoftObjectPtr OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithSoftObjectPtr::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithSoftObjectPtr::StaticStruct(), &OutStruct)
	);


	if (0)
	{
		//	only checks for compilation
		//////////////////////
		FDcReader Reader;
		Reader.ReadWeakObjectField(&Struct.Weak1);
		Reader.ReadLazyObjectField(&Struct.Lazy1);
		Reader.ReadSoftObjectField(&Struct.Soft1);
		Reader.ReadSoftClassField(&Struct.Soft2);

		//////////////////////
		FDcWriter Writer;
		Writer.WriteWeakObjectField(Struct.Weak1);
		Writer.WriteLazyObjectField(Struct.Lazy1);
		Writer.WriteSoftObjectField(Struct.Soft1);
		Writer.WriteSoftClassField(Struct.Soft2);
	}
}

FString IFooInterface::GetFooName()
{
	return FooStr;
}

UInterfacedAlpha::UInterfacedAlpha()
{
	FooStr = TEXT("I cover the waterfront.");
}

UInterfacedBeta::UInterfacedBeta()
{
	FooStr = TEXT("I'm watching the sea");
}


void PropertyVisitorRoundtrip_ScriptInterface()
{
	FStructWithInterface Struct;

	UObject* Alpha = NewObject<UInterfacedAlpha>();
	Struct.FooInterface = Alpha;

	FStructWithInterface OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithInterface::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithInterface::StaticStruct(), &OutStruct)
	);

	if (0)
	{
		//	only checks for compilation
		//////////////////////
		FDcReader Reader;
		Reader.ReadInterfaceField(&Struct.FooInterface);

		//////////////////////
		FDcWriter Writer;
		Writer.WriteInterfaceField(Struct.FooInterface);
	}
}


void PropertyVisitorRoundtrip_Delegates()
{
	UDelegateClass* Cls = NewObject<UDelegateClass>();
	FStructWithDelegate Struct;

	Struct.Delegate1.BindDynamic(Cls, &UDelegateClass::ReturnOne);
	Struct.DelgateGroup2.AddDynamic(Cls, &UDelegateClass::ReturnNone);

	FStructWithDelegate OutStruct{};

	_Roundtrip(
		FDcPropertyDatum(FStructWithDelegate::StaticStruct(), &Struct),
		FDcPropertyDatum(FStructWithDelegate::StaticStruct(), &OutStruct)
	);

	Cls->SparseCallback1.AddDynamic(Cls, &UDelegateClass::ReturnNone);

	UDelegateClass* OutCls = NewObject<UDelegateClass>();

	_Roundtrip(
		FDcPropertyDatum(UDelegateClass::StaticClass(), Cls),
		FDcPropertyDatum(UDelegateClass::StaticClass(), OutCls)
	);


	if (0)
	{
		//////////////////////
		FDcReader Reader;
		Reader.ReadDelegateField(&Struct.Delegate1);

		//////////////////////
	}

}

int UDelegateClass::ReturnOne(int Int)
{
	return Int;
}

void UDelegateClass::ReturnNone(int Int)
{
	/* pass */
}
