#include "Adhocs.h"
#include "Reader/PropertyReader.h"
#include "UObject/UnrealType.h"

class FScafVisitor : public FVisitor
{
	FVisitResult VisitBool(bool Value) override
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Visiting a bool: %d"), Value);
		return FVisitResult::Ok();
	}

	FVisitResult VisitName(FName Name) override
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Visiting a name: %s"), *Name.ToString());
		return FVisitResult::Ok();
	}

	FVisitResult VisitString(FString Str) override
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Visiting a str: %s"), *Str);
		return FVisitResult::Ok();
	}

	FVisitResult VisitMap(FMapAccess& MapAccess) override
	{
		while (true)
		{
			bool bHasPending;
			FVisitResult Ret;

			//	TODO need atleast a try macro
			//		 this is also why rust error handling is superior
			Ret = MapAccess.HasPending(bHasPending);
			if (!bHasPending) return Ret;
			if (!Ret.bOK) return Ret;

			Ret = MapAccess.ReadKey(*this);
			if (!Ret.bOK) return Ret;
			Ret = MapAccess.ReadValue(*this);
			if (!Ret.bOK) return Ret;
			Ret = MapAccess.Next();
			if (!Ret.bOK) return Ret;
		}

		return FVisitResult::Ok();
	}

};

void PropertyReaderScaffolding()
{
	FScafVisitor Visitor;

	FTestStruct_Alpha StructAlpha{};
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));

	FPropertyReader Reader(
		(void*)&StructAlpha,
		FTestStruct_Alpha::StaticStruct()
	);

	Reader.ReadAny(Visitor);

	return;
}
