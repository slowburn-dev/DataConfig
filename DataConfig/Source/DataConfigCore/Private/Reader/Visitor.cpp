#include "Reader/Visitor.h"

FVisitor::~FVisitor()
{}

FVisitResult FVisitor::VisitBool(bool Value)
{
	return FVisitResult::Ok();
}
