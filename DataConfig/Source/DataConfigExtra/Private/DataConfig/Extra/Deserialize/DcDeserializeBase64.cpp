#include "DataConfig/Extra/Deserialize/DcDeserializeBase64.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "Misc/Base64.h"

namespace DcExtra
{

EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx)
{
	FArrayProperty* ArrayProperty = DcPropertyUtils::CastFieldVariant<FArrayProperty>(Ctx.TopProperty());

	//	check for only TArray<uint8>
	if (ArrayProperty == nullptr)
		return EDcDeserializePredicateResult::Pass;
	if (!ArrayProperty->Inner->IsA<FByteProperty>())
		return EDcDeserializePredicateResult::Pass;

	bool bIsMetaBlob;
	//	TODO use amend meta data
#if WITH_METADATA
	bIsMetaBlob = ArrayProperty->HasMetaData(TEXT("DcExtraBlob"));
#else
	bIsMetaBlob = ArrayProperty->GetName().EndsWith(TEXT("Blob"));
#endif

	return bIsMetaBlob
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::String;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::Blob, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	FString Base64Str;
	DC_TRY(Ctx.Reader->ReadString(&Base64Str));

	TArray<uint8> Buffer;
	bool bOk = FBase64::Decode(Base64Str, Buffer);

	if (!bOk)
		return DC_FAIL(DcDExtra, InvalidBase64String);

	DC_TRY(Ctx.Writer->WriteBlob(FDcBlobViewData{ Buffer.GetData(), Buffer.Num() }));

	return DcOkWithProcessed(OutRet);
}

}	//	namespace DcExtra



