#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/DcEnv.h"

#include "Misc/Base64.h"

namespace DcExtra
{

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx)
{
	FArrayProperty* ArrayProperty = DcPropertyUtils::CastFieldVariant<FArrayProperty>(Ctx.TopProperty());

	//	check for only TArray<uint8>
	if (ArrayProperty == nullptr)
		return EDcDeserializePredicateResult::Pass;
	if (!ArrayProperty->Inner->IsA<FByteProperty>())
		return EDcDeserializePredicateResult::Pass;

	bool bIsMetaData;
#if WITH_METADATA
	bIsMetaData = ArrayProperty->HasMetaData(TEXT("DcExtraBlob"));
#else
	bIsMetaData = ArrayProperty->GetName().EndsWith(TEXT("Blob"));
#endif

	return bIsMetaData
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

DATACONFIGEXTRA_API FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
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

	//	TODO [DIAG] user diagnostic message, find a way to insert it.
	if (!bOk) return DcFail();

	DC_TRY(Ctx.Writer->WriteBlob(FDcBlobViewData{ Buffer.GetData(), Buffer.Num() }));

	return DcOkWithProcessed(OutRet);
}

}	//	namespace DcExtra

