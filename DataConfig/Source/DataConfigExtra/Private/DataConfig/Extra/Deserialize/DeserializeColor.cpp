#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Extra/Deserialize/DeserializeColor.h"

namespace DcExtra
{

EDcDeserializePredicateResult DATACONFIGEXTRA_API PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Ctx.TopProperty()))
	{
		return StructProperty->Struct->GetFName() == TEXT("Color")
			? EDcDeserializePredicateResult::Process
			: EDcDeserializePredicateResult::Pass;
	}
	else if (UScriptStruct* StructClass = Cast<UScriptStruct>(Ctx.TopProperty()))
	{
		return StructClass->GetFName() == TEXT("Color")
			? EDcDeserializePredicateResult::Process
			: EDcDeserializePredicateResult::Pass;
	}
	else
	{
		return EDcDeserializePredicateResult::Pass;
	}
}

FDcResult DATACONFIGEXTRA_API HandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::String;

	//	! just noticed that PeekWrite API is actually so fucking wrong.
	//Ctx.Writer->PeekWrite

	return DcOk();
}


}	//	namespace DcExtra


