#include "Reader/Visitor.h"

FVisitor::~FVisitor()
{}

FVisitResult FVisitor::VisitBool(bool Value)
{
	return FVisitResult::Fail(TEXT("Unexpected bool"));
}

FVisitResult FVisitor::VisitName(FName Name)
{
	return FVisitResult::Fail(TEXT("Unexpected Name"));
}

FVisitResult FVisitor::VisitString(FString Str)
{
	return FVisitResult::Fail(TEXT("Unexpected String"));
}

FVisitResult FVisitor::VisitMap(FMapAccess& MapAccess)
{
	return FVisitResult::Fail(TEXT("Unexpected MapAccess"));
}


FVisitResult FMapAccess::Num(size_t& OutNum)
{
	return FVisitResult::Fail(TEXT("Unexpected MapAccess Num"));
}

FVisitResult FMapAccess::HasPending(bool& bOutHasPending)
{
	return FVisitResult::Fail(TEXT("Unexpected MapAccess HasPending"));
}

FVisitResult FMapAccess::ReadKey(FVisitor &Visitor)
{
	return FVisitResult::Fail(TEXT("Unexpected Key"));
}

FVisitResult FMapAccess::ReadValue(FVisitor &Visitor)
{
	return FVisitResult::Fail(TEXT("Unexpected Value"));
}

FVisitResult FMapAccess::Next()
{
	return FVisitResult::Fail(TEXT("Unexpected Next"));
}

FMapAccess::~FMapAccess()
{}
