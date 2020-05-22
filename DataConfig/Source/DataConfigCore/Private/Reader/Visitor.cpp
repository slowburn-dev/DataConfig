#include "Reader/Visitor.h"

namespace DataConfig {

FVisitor::~FVisitor()
{}

FResult FVisitor::VisitBool(bool Value) { return Fail(EErrorCode::UnexpectedBool); }
FResult FVisitor::VisitName(const FName& Name) { return Fail(EErrorCode::UnexpectedName); }
FResult FVisitor::VisitString(const FString& Str) { return Fail(EErrorCode::UnexpectedString); }
FResult FVisitor::VisitFloat(float Value) { return Fail(EErrorCode::UnexpectedFloat); }
FResult FVisitor::VisitDouble(double Value) { return Fail(EErrorCode::UnexpectedDouble); }
FResult FVisitor::VisitInt8(int8 Value) { return Fail(EErrorCode::UnexpectedInt8); }
FResult FVisitor::VisitInt16(int16 Value) { return Fail(EErrorCode::UnexpectedInt16); }
FResult FVisitor::VisitInt(int Value) { return Fail(EErrorCode::UnexpectedInt); }
FResult FVisitor::VisitInt64(int64 Value) { return Fail(EErrorCode::UnexpectedInt64); }
FResult FVisitor::VisitByte(uint8 Value) { return Fail(EErrorCode::UnexpectedByte); }
FResult FVisitor::VisitUInt16(uint16 Value) { return Fail(EErrorCode::UnexpectedUInt16); }
FResult FVisitor::VisitUInt32(uint32 Value) { return Fail(EErrorCode::UnexpectedUInt32); }
FResult FVisitor::VisitUInt64(uint64 Value) { return Fail(EErrorCode::UnexpectedUInt64); }
FResult FVisitor::VisitStruct(const FName& StructName, FMapAccess& MapAccess) { return Fail(EErrorCode::UnexpectedStruct); }
FResult FVisitor::VisitClass(const FName& ClassName, FMapAccess& MapAccess) { return Fail(EErrorCode::UnexpectedClass); }
FResult FVisitor::VisitMap(FMapAccess& MapAccess) { return Fail(EErrorCode::UnexpectedMap); }

FResult FMapAccess::Num(TOptional<size_t>& OutNum) { return Fail(EErrorCode::UnexpectedMapAccess); }
FResult FMapAccess::HasPending(bool& bOutHasPending) { return Fail(EErrorCode::UnexpectedMapAccess); }
FResult FMapAccess::ReadKey(FVisitor &Visitor) { return Fail(EErrorCode::UnexpectedMapAccess); }
FResult FMapAccess::ReadValue(FVisitor &Visitor) { return Fail(EErrorCode::UnexpectedMapAccess); }
FResult FMapAccess::Next() { return Fail(EErrorCode::UnexpectedMapAccess);}
FMapAccess::~FMapAccess() {}

} // namespace DataConfig

