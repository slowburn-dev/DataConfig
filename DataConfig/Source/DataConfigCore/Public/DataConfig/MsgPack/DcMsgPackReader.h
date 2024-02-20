#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DcMsgPackUtils.h"
#include "DataConfig/Reader/DcReader.h"

struct DATACONFIGCORE_API FDcMsgPackReader : public FDcReader, private FNoncopyable
{
	FDcMsgPackReader();
	FDcMsgPackReader(FDcBlobViewData Blob);

	FDcBlobViewData View;

	enum class EReadState : uint8
	{
		Root,
		Map,
		Array,
	};

	struct FReadState
	{
		EReadState Type;
		bool bMapAtValue;

		int32 Remain;
	};
	TArray<FReadState, TInlineAllocator<8>> States;

	struct FState
	{
		uint8 bNeedReadNext : 1;
		uint8 LastTypeByte;

		int Index;
		FORCEINLINE void Reset() { *this = FState{}; }
	};
	FState State = {};

	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult Coercion(EDcDataEntry ToEntry, bool* OutPtr) override;

	FDcResult ReadNone() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadText(FText* OutPtr) override;

	FDcResult ReadBlob(FDcBlobViewData* OutPtr) override;

	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;
	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;

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

	FDcResult PeekTypeByte(uint8* OutPtr);

	FDcResult ReadFixExt1(uint8* OutType, uint8* OutByte);
	FDcResult ReadFixExt2(uint8* OutType, FDcBytes2* OutBytes);
	FDcResult ReadFixExt4(uint8* OutType, FDcBytes4* OutBytes);
	FDcResult ReadFixExt8(uint8* OutType, FDcBytes8* OutBytes);
	FDcResult ReadFixExt16(uint8* OutType, FDcBytes16* OutBytes);

	FDcResult ReadExt(uint8* OutType, FDcBlobViewData* OutBlob);

	static FName ClassId();
	FName GetId() override;

	void FormatDiagnostic(FDcDiagnostic& Diag) override;

};


