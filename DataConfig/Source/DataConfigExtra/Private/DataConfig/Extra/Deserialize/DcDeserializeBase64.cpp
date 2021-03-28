#include "DataConfig/Extra/Deserialize/DcDeserializeBase64.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
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

	return ArrayProperty->HasMetaData(TEXT("DcExtraBase64"))
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx)
{
	FString Base64Str;
	DC_TRY(Ctx.Reader->ReadString(&Base64Str));

	TArray<uint8> Buffer;
	bool bOk = FBase64::Decode(Base64Str, Buffer);

	if (!bOk)
		return DC_FAIL(DcDExtra, InvalidBase64String);

	DC_TRY(Ctx.Writer->WriteBlob(FDcBlobViewData{ Buffer.GetData(), Buffer.Num() }));

	return DcOk();
}

} // namespace DcExtra


DC_TEST("DataConfig.Extra.Deserialize.Base64")
{
	using namespace DcExtra;
	FDcExtraTestStructWithBase64 Dest;
	FDcPropertyDatum DestDatum(FDcExtraTestStructWithBase64::StaticStruct(), &Dest);

	FString Str = TEXT(R"(
		{
			"BlobField1" : "dGhlc2UgYXJlIG15IHR3aXN0ZWQgd29yZHM=",
			"BlobField2" : "",
		}
	)");
	FDcJsonReader Reader(Str);

	FDcExtraTestStructWithBase64 Expect;

	const char* Literal = "these are my twisted words";
	Expect.BlobField1 = TArray<uint8>((uint8*)Literal, FCStringAnsi::Strlen(Literal));
	Expect.BlobField2 = TArray<uint8>();

	FDcPropertyDatum ExpectDatum(FDcExtraTestStructWithBase64::StaticStruct(), &Expect);

#if !WITH_METADATA
	DcAutomationUtils::AmendMetaData(FDcExtraTestStructWithBase64::StaticStruct(), TEXT("BlobField1"), TEXT("DcExtraBase64"), TEXT(""));
	DcAutomationUtils::AmendMetaData(FDcExtraTestStructWithBase64::StaticStruct(), TEXT("BlobField2"), TEXT("DcExtraBase64"), TEXT(""));
#endif

	UTEST_OK("Extra Base64 Blob Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsBase64Blob),
			FDcDeserializeDelegate::CreateStatic(HandleBase64BlobDeserialize)
		);
	}));

	UTEST_OK("Extra Base64 Blob Deserialize", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	FString BlobField1AsStr(Dest.BlobField1.Num(), (ANSICHAR*)Dest.BlobField1.GetData());
	UTEST_EQUAL("Extra Base64 Blob Deserialize", "these are my twisted words", BlobField1AsStr);
	UTEST_EQUAL("Extra Base64 Blob Deserialize", Dest.BlobField2.Num(), 0);



	return true;
}

