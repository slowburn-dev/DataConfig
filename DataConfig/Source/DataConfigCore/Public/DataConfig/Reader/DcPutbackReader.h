#pragma once

#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Misc/DcDataVariant.h"

struct DATACONFIGCORE_API FDcPutbackReader : public FDcReader
{
	FDcPutbackReader(FDcReader* InReader)
		: Reader(InReader)
	{}

	FDcResult PeekRead(EDcDataEntry* OutPtr) override;

	FDcResult ReadNone() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadText(FText* OutPtr) override;
	FDcResult ReadEnum(FDcEnumData* OutPtr) override;

	FDcResult ReadStructRootAccess(FDcStructAccess& Access) override;
	FDcResult ReadStructEndAccess(FDcStructAccess& Access) override;
	FDcResult ReadClassRootAccess(FDcClassAccess& Access) override;
	FDcResult ReadClassEndAccess(FDcClassAccess& Access) override;
	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;
	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;
	FDcResult ReadSetRoot() override;
	FDcResult ReadSetEnd() override;
	FDcResult ReadOptionalRoot() override;
	FDcResult ReadOptionalEnd() override;

	FDcResult ReadObjectReference(UObject** OutPtr) override;
	FDcResult ReadClassReference(UClass** OutPtr) override;

	FDcResult ReadWeakObjectReference(FWeakObjectPtr* OutPtr) override;
	FDcResult ReadLazyObjectReference(FLazyObjectPtr* OutPtr) override;
	FDcResult ReadSoftObjectReference(FSoftObjectPtr* OutPtr) override;
	FDcResult ReadSoftClassReference(FSoftObjectPtr* OutPtr) override;
	FDcResult ReadInterfaceReference(FScriptInterface* OutPtr) override;

	FDcResult ReadFieldPath(FFieldPath* OutPtr) override;
	FDcResult ReadDelegate(FScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastSparseDelegate(FMulticastScriptDelegate* OutPtr) override;

	FDcResult ReadInt8(int8* OutPtr) override;
	FDcResult ReadInt16(int16* OutPtr) override;
	FDcResult ReadInt32(int32* OutPtr) override;
	FDcResult ReadInt64(int64* OutPtr) override;

	FDcResult ReadUInt8(uint8* OutPtr) override;
	FDcResult ReadUInt16(uint16* OutPtr) override;
	FDcResult ReadUInt32(uint32* OutPtr) override;
	FDcResult ReadUInt64(uint64* OutPtr) override;

	FDcResult ReadFloat(float* OutPtr) override;
	FDcResult ReadDouble(double* OutPtr) override;

	FDcResult ReadBlob(FDcBlobViewData* OutPtr) override;

	template<typename T>
	void Putback(T&& InValue);

	TArray<FDcDataVariant> Cached;
	FDcReader* Reader;

	FDcResult Coercion(EDcDataEntry ToEntry, bool* OutPtr) override;
	void FormatDiagnostic(FDcDiagnostic& Diag) override;

	static FName ClassId();
	FName GetId() override;

};

template<typename T>
void FDcPutbackReader::Putback(T&& InValue)
{
	Cached.Insert(Forward<T>(InValue), 0);
}

