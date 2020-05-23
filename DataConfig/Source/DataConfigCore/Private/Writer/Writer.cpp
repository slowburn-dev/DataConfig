#include "Writer/Writer.h"

namespace DataConfig
{

FWriter::FWriter()
{}

FWriter::~FWriter()
{}

FResult FWriter::WriteBool(bool Value) { return Fail(EErrorCode::UnexpectedBool); }
FResult FWriter::WriteName(const FName& Value) { return Fail(EErrorCode::UnexpectedName); }
FResult FWriter::WriteString(const FString& Value) { return Fail(EErrorCode::UnexpectedString); }
FResult FWriter::WriteFloat(float Value) { return Fail(EErrorCode::UnexpectedFloat); }
FResult FWriter::WriteDouble(double Value) { return Fail(EErrorCode::UnexpectedDouble); }
FResult FWriter::WriteInt8(int8 Value) { return Fail(EErrorCode::UnexpectedInt8);}
FResult FWriter::WriteInt16(int16 Value) { return Fail(EErrorCode::UnexpectedInt16); }
FResult FWriter::WriteInt(int Value) { return Fail(EErrorCode::UnexpectedInt); }
FResult FWriter::WriteInt64(int64 Value) { return Fail(EErrorCode::UnexpectedInt64); }
FResult FWriter::WriteByte(uint8 Value) { return Fail(EErrorCode::UnexpectedByte); }
FResult FWriter::WriteUInt16(uint16 Value) { return Fail(EErrorCode::UnexpectedUInt16); }
FResult FWriter::WriteUInt32(uint32 Value) { return Fail(EErrorCode::UnexpectedUInt32); }
FResult FWriter::WriteUInt64(uint64 Value) { return Fail(EErrorCode::UnexpectedUInt64); }
FResult FWriter::WriteStruct(const FName& StructName, FWriterStorage& OutWriter) { return Fail(EErrorCode::UnexpectedStruct); }
FResult FWriter::WriteClass(const FName& ClassName, FWriterStorage& OutWriter) { return Fail(EErrorCode::UnexpectedClass); }
FResult FWriter::WriteMap(FWriterStorage& OutWriter) { return Fail(EErrorCode::UnexpectedMap); }
FResult FWriter::End() { return Fail(EErrorCode::UnexpectedEnd); }

FWriterStorage::FWriterStorage()
{
	FMemory::Memzero(ImplStorage.Data, sizeof(FWriterStorage));
}

FWriterStorage::~FWriterStorage()
{
	if (!IsEmpty())
	{
		Get()->~FWriter();
	}
}

bool FWriterStorage::IsEmpty()
{
	return FMemory::Memcmp(ImplStorage.Data, GetZeroBitPattern128(), sizeof(FWriterStorage)) == 0;
}


}	// namespace DataConfig


