#include "Reader/Visitor.h"
#include "Reader/ReaderErrorCodes.h"

namespace DataConfig {

FVisitor::~FVisitor()
{}

FVisitResult FVisitor::VisitBool(bool Value)
{
	return Fail(EReaderErrorCode::UnexpectedBool);
}

FVisitResult FVisitor::VisitName(FName Name)
{
	return Fail(EReaderErrorCode::UnexpectedName);
}

FVisitResult FVisitor::VisitString(FString Str)
{
	return Fail(EReaderErrorCode::UnexpectedString);
}

FVisitResult FVisitor::VisitMap(FMapAccess& MapAccess)
{
	return Fail(EReaderErrorCode::UnexpectedMap);
}

FVisitResult FMapAccess::Num(size_t& OutNum)
{
	return Fail(EReaderErrorCode::UnexpectedMapNum);
}

FVisitResult FMapAccess::HasPending(bool& bOutHasPending)
{
	return Fail(EReaderErrorCode::UnexpectedMapHasPending);
}

FVisitResult FMapAccess::ReadKey(FVisitor &Visitor)
{
	return Fail(EReaderErrorCode::UnexpectedMapKey);
}

FVisitResult FMapAccess::ReadValue(FVisitor &Visitor)
{
	return Fail(EReaderErrorCode::UnexpectedMapValue);
}

FVisitResult FMapAccess::Next()
{
	return Fail(EReaderErrorCode::UnexpectedMapNext);
}

FMapAccess::~FMapAccess()
{}

} // namespace DataConfig

