#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/DcEnv.h"

#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.h"

namespace DcExtra
{
EDcDeserializePredicateResult DATACONFIGEXTRA_API PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Ctx.TopProperty()))
	{
		return StructProperty->Struct == FDcAnyStruct::StaticStruct()
			? EDcDeserializePredicateResult::Process
			: EDcDeserializePredicateResult::Pass;
	}
	else
	{
		return EDcDeserializePredicateResult::Pass;
	}
}

FDcResult DATACONFIGEXTRA_API HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithCanNotProcess(OutRet);

	DC_TRY(Ctx.Reader->ReadMapRoot());
	FString Str;
	DC_TRY(Ctx.Reader->ReadString(&Str));
	if (!DcDeserializeUtils::IsMeta(Str))
		return DC_FAIL(DcDDeserialize, ExpectMetaType);

	DC_TRY(Ctx.Reader->ReadString(&Str));
	UScriptStruct* LoadStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *Str, true);
	if (LoadStruct == nullptr)
		return DC_FAIL(DcDDeserialize, UObjectByNameNotFound) << MoveTemp(Str);

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(UStructProperty::StaticClass(), Datum));

	FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)Datum.DataPtr;
	AnyStructPtr->DataPtr = (uint8*)FMemory::Malloc(LoadStruct->GetStructureSize());
	LoadStruct->InitializeStruct(AnyStructPtr->DataPtr);
	AnyStructPtr->StructClass = LoadStruct;

	Ctx.Writer->PushTopStructPropertyState({LoadStruct, (void*)AnyStructPtr->DataPtr}, Ctx.TopProperty()->GetFName());

	//	TODO can never get it right
	//		 need to swap the reader right in here, and continue on reading, then switch the reader back

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
		{
			DC_TRY(Ctx.Reader->ReadMapEnd());
			break;
		}
		else if (CurPeek == EDcDataEntry::String)
		{
			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));
			DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		}

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	return DcOkWithProcessed(OutRet);
}

}	//	namespace DcExtra

