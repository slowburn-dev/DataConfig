#include "Reader/Visitor.h"
#include "Reader/ReaderErrorCodes.h"

namespace DataConfig {

FVisitor::~FVisitor()
{}

FVisitResult FVisitor::VisitBool(bool Value) { return Fail(EReaderErrorCode::UnexpectedBool); }
FVisitResult FVisitor::VisitName(const FName& Name) { return Fail(EReaderErrorCode::UnexpectedName); }
FVisitResult FVisitor::VisitString(const FString& Str) { return Fail(EReaderErrorCode::UnexpectedString); }
FVisitResult FVisitor::VisitFloat(float Value) { return Fail(EReaderErrorCode::UnexpectedFloat); }
FVisitResult FVisitor::VisitDouble(double Value) { return Fail(EReaderErrorCode::UnexpectedDouble); }
FVisitResult FVisitor::VisitInt8(int8 Value) { return Fail(EReaderErrorCode::UnexpectedInt8); }
FVisitResult FVisitor::VisitInt16(int16 Value) { return Fail(EReaderErrorCode::UnexpectedInt16); }
FVisitResult FVisitor::VisitInt(int Value) { return Fail(EReaderErrorCode::UnexpectedInt); }
FVisitResult FVisitor::VisitInt64(int64 Value) { return Fail(EReaderErrorCode::UnexpectedInt64); }
FVisitResult FVisitor::VisitByte(uint8 Value) { return Fail(EReaderErrorCode::UnexpectedByte); }
FVisitResult FVisitor::VisitUInt16(uint16 Value) { return Fail(EReaderErrorCode::UnexpectedUInt16); }
FVisitResult FVisitor::VisitUInt32(uint32 Value) { return Fail(EReaderErrorCode::UnexpectedUInt32); }
FVisitResult FVisitor::VisitUInt64(uint64 Value) { return Fail(EReaderErrorCode::UnexpectedUInt64); }
FVisitResult FVisitor::VisitStruct(const FName& StructName, FMapAccess& MapAccess) { return Fail(EReaderErrorCode::UnexpectedStruct); }
FVisitResult FVisitor::VisitClass(const FName& ClassName, FMapAccess& MapAccess) { return Fail(EReaderErrorCode::UnexpectedClass); }
FVisitResult FVisitor::VisitMap(FMapAccess& MapAccess) { return Fail(EReaderErrorCode::UnexpectedMap); }

FVisitResult FMapAccess::Num(TOptional<size_t>& OutNum) { return Fail(EReaderErrorCode::UnexpectedMapAccess); }
FVisitResult FMapAccess::HasPending(bool& bOutHasPending) { return Fail(EReaderErrorCode::UnexpectedMapAccess); }
FVisitResult FMapAccess::ReadKey(FVisitor &Visitor) { return Fail(EReaderErrorCode::UnexpectedMapAccess); }
FVisitResult FMapAccess::ReadValue(FVisitor &Visitor) { return Fail(EReaderErrorCode::UnexpectedMapAccess); }
FVisitResult FMapAccess::Next() { return Fail(EReaderErrorCode::UnexpectedMapAccess);}
FMapAccess::~FMapAccess() {}

} // namespace DataConfig

