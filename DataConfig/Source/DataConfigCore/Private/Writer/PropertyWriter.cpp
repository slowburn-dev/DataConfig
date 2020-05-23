#include "Writer/PropertyWriter.h"

namespace DataConfig {

FPropertyWriter::FPropertyWriter()
{
}

FPropertyWriter::FPropertyWriter(FPropertyDatum Datum)
{
}

FResult FPropertyWriter::Peek(EDataEntry Next)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyWriter::WriteBool(bool Value)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyWriter::WriteName(const FName& Value)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyWriter::WriteString(const FString& Value)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyWriter::WriteStructRoot(const FName& Name)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyWriter::WriteStructEnd(const FName& Name)
{
	return Fail(EErrorCode::UnknownError);
}

} // namespace DataConfig
