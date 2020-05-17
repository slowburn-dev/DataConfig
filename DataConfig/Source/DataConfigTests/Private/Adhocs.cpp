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
};


void PropertyReaderScaffolding()
{
	//	still cann't create on the stack property
	//	maybe it's used for like ScriptArray
	/*
	FScafVisitor Visitor;

	bool bValue = true;
	FObjectInitializer AdhocInitializer;
	UBoolProperty Property(AdhocInitializer);
	FPropertyReader Reader(&bValue, &Property);
	Reader.ReadBool(Visitor);
	*/

	return;
}
