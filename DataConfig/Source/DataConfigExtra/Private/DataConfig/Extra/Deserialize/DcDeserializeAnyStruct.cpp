#include "DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DataConfig/Reader/DcPutbackReader.h"

namespace DcExtra
{
EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx)
{
	if (FStructProperty* StructProperty = DcPropertyUtils::CastFieldVariant<FStructProperty>(Ctx.TopProperty()))
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

FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

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
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	void* DataPtr = (uint8*)FMemory::Malloc(LoadStruct->GetStructureSize());
	LoadStruct->InitializeStruct(DataPtr);
	FDcAnyStruct TmpAny{DataPtr, LoadStruct};

	FDcAnyStruct* AnyStructPtr = (FDcAnyStruct*)Datum.DataPtr;
	*AnyStructPtr = MoveTemp(TmpAny);

	Ctx.Writer->PushTopStructPropertyState({LoadStruct, (void*)AnyStructPtr->DataPtr}, Ctx.TopProperty().GetFName());

	FDcPutbackReader PutbackReader(Ctx.Reader);
	PutbackReader.Putback(EDcDataEntry::MapRoot);
	TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

	FDcScopedProperty ScopedValueProperty(Ctx);
	DC_TRY(ScopedValueProperty.PushProperty());
	DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

	return DcOkWithProcessed(OutRet);
}

}	//	namespace DcExtra

