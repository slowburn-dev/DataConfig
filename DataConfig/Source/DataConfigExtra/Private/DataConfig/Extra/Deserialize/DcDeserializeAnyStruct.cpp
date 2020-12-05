#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
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

	//FDcPropertyDatum Datum{LoadStruct, 



	return DcOk();
}

}	//	namespace DcExtra

