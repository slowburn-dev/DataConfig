#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"
#include "DataConfig/Writer/DcWriter.h"

struct DATACONFIGCORE_API FDcMsgPackWriter : public FDcWriter, private FNoncopyable
{
	using BufferType = TArray<uint8, TInlineAllocator<1024>>;

	enum class EWriteState : uint8
	{
		Root,
		Map,
		Array,
	};

	struct FWriteState
	{
		EWriteState Type;
		bool bMapAtValue;
		uint8 LastTypeByte;

		uint32 Size;
		BufferType Buffer;
	};

	TArray<FWriteState, TInlineAllocator<4>> States;
	FORCEINLINE EWriteState GetTopStateType() { return States.Top().Type; }

	BufferType& GetMainBuffer();

	FDcMsgPackWriter();

	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;

	FDcResult WriteNil() override;
	FDcResult WriteBool(bool Value) override;
	FDcResult WriteString(const FString& Value) override;
	FDcResult WriteName(const FName& Name) override;
	FDcResult WriteText(const FText& Value) override;

	FDcResult WriteBlob(const FDcBlobViewData& Value) override;

	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;
	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;

	FDcResult WriteInt8(const int8& Value) override;
	FDcResult WriteInt16(const int16& Value) override;
	FDcResult WriteInt32(const int32& Value) override;
	FDcResult WriteInt64(const int64& Value) override;

	FDcResult WriteUInt8(const uint8& Value) override;
	FDcResult WriteUInt16(const uint16& Value) override;
	FDcResult WriteUInt32(const uint32& Value) override;
	FDcResult WriteUInt64(const uint64& Value) override;

	FDcResult WriteFloat(const float& Value) override;
	FDcResult WriteDouble(const double& Value) override;

	FDcResult WriteFixExt1(uint8 Type, uint8 Byte);
	FDcResult WriteFixExt2(uint8 Type, FDcBytes2 Bytes);
	FDcResult WriteFixExt4(uint8 Type, FDcBytes4 Bytes);
	FDcResult WriteFixExt8(uint8 Type, FDcBytes8 Bytes);
	FDcResult WriteFixExt16(uint8 Type, FDcBytes16 Bytes);

	FDcResult WriteExt(uint8 Type, FDcBlobViewData Blob);

	void FormatDiagnostic(FDcDiagnostic& Diag) override;

	static FName ClassId(); 
	FName GetId() override;
};



