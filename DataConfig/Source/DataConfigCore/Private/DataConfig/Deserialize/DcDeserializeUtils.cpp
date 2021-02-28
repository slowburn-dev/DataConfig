#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"

namespace DcDeserializeUtils {

FString DC_STR_META_TYPE = FString(TEXT("$type"));
FString DC_STR_META_PATH = FString(TEXT("$path"));

FName DC_NAME_META_TYPE = FName(TEXT("$type"));
FName DC_NAME_META_PATH = FName(TEXT("$path"));

const TBasicArray<FName>& DcGetAllNameMetas()
{
	static TBasicArray<FName> ALL_NAME_METAS;
	if (ALL_NAME_METAS.Num() == 0)
	{
		ALL_NAME_METAS.Emplace(DC_NAME_META_TYPE);
		ALL_NAME_METAS.Emplace(DC_NAME_META_PATH);
	}
	return ALL_NAME_METAS;
}


const TBasicArray<FString>& GetAllStringMetas()
{
	static TBasicArray<FString> ALL_STR_METAS;
	if (ALL_STR_METAS.Num() == 0)
	{
		ALL_STR_METAS.Emplace(DC_STR_META_TYPE);
		ALL_STR_METAS.Emplace(DC_STR_META_PATH);
	}
	return ALL_STR_METAS;
}

bool IsMeta(const FName& Name)
{
	for (const FName& Cur : DcGetAllNameMetas())
	{
		if (Name == Cur)
			return true;
	}

	return false;
}

bool IsMeta(const FString& Str)
{
	for (const FString& Cur : GetAllStringMetas())
	{
		if (Str == Cur)
			return true;
	}

	return false;
}

FDcResult DispatchPipeVisit(EDcDataEntry Next, FDcReader* Reader, FDcWriter* Writer)
{
	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Reader->ReadNil());
		DC_TRY(Writer->WriteNil());
	}
	else if (Next == EDcDataEntry::Bool)
	{
		bool Value;
		DC_TRY(Reader->ReadBool(&Value));
		DC_TRY(Writer->WriteBool(Value));
	}
	else if (Next == EDcDataEntry::Name)
	{
		FName Value;
		DC_TRY(Reader->ReadName(&Value));
		DC_TRY(Writer->WriteName(Value));
	}
	else if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Reader->ReadString(&Value));
		DC_TRY(Writer->WriteString(Value));
	}
	else if (Next == EDcDataEntry::Text)
	{
		FText Value;
		DC_TRY(Reader->ReadText(&Value));
		DC_TRY(Writer->WriteText(Value));
	}
	else if (Next == EDcDataEntry::Enum)
	{
		FDcEnumData Value;
		DC_TRY(Reader->ReadEnum(&Value));
		DC_TRY(Writer->WriteEnum(Value));
	}
	else if (Next == EDcDataEntry::StructRoot)
	{
		FDcStructStat StructStat;
		DC_TRY(Reader->ReadStructRoot(&StructStat));
		DC_TRY(Writer->WriteStructRoot(StructStat));
	}
	else if (Next == EDcDataEntry::StructEnd)
	{
		FDcStructStat StructStat;
		DC_TRY(Reader->ReadStructEnd(&StructStat));
		DC_TRY(Writer->WriteStructEnd(StructStat));
	}
	else if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Reader->ReadMapRoot());
		DC_TRY(Writer->WriteMapRoot());
	}
	else if (Next == EDcDataEntry::MapEnd)
	{
		DC_TRY(Reader->ReadMapEnd());
		DC_TRY(Writer->WriteMapEnd());
	}
	else if (Next == EDcDataEntry::ClassRoot)
	{
		FDcClassStat ClassStat;
		DC_TRY(Reader->ReadClassRoot(&ClassStat));
		DC_TRY(Writer->WriteClassRoot(ClassStat));
	}
	else if (Next == EDcDataEntry::ClassEnd)
	{
		FDcClassStat ClassStat;
		DC_TRY(Reader->ReadClassEnd(&ClassStat));
		DC_TRY(Writer->WriteClassEnd(ClassStat));
	}
	else if (Next == EDcDataEntry::ArrayRoot)
	{
		DC_TRY(Reader->ReadArrayRoot());
		DC_TRY(Writer->WriteArrayRoot());
	}
	else if (Next == EDcDataEntry::ArrayEnd)
	{
		DC_TRY(Reader->ReadArrayEnd());
		DC_TRY(Writer->WriteArrayEnd());
	}
	else if (Next == EDcDataEntry::SetRoot)
	{
		DC_TRY(Reader->ReadSetRoot());
		DC_TRY(Writer->WriteSetRoot());
	}
	else if (Next == EDcDataEntry::SetEnd)
	{
		DC_TRY(Reader->ReadSetEnd());
		DC_TRY(Writer->WriteSetEnd());
	}
	else if (Next == EDcDataEntry::ObjectReference)
	{
		UObject* Value;
		DC_TRY(Reader->ReadObjectReference(&Value));
		DC_TRY(Writer->WriteObjectReference(Value));
	}
	else if (Next == EDcDataEntry::ClassReference)
	{
		UClass* Value;
		DC_TRY(Reader->ReadClassReference(&Value));
		DC_TRY(Writer->WriteClassReference(Value));
	}
	else if (Next == EDcDataEntry::WeakObjectReference)
	{
		FWeakObjectPtr Value;
		DC_TRY(Reader->ReadWeakObjectReference(&Value));
		DC_TRY(Writer->WriteWeakObjectReference(Value));
	}
	else if (Next == EDcDataEntry::LazyObjectReference)
	{
		FLazyObjectPtr Value;
		DC_TRY(Reader->ReadLazyObjectReference(&Value));
		DC_TRY(Writer->WriteLazyObjectReference(Value));
	}
	else if (Next == EDcDataEntry::SoftObjectReference)
	{
		FSoftObjectPath Value;
		DC_TRY(Reader->ReadSoftObjectReference(&Value));
		DC_TRY(Writer->WriteSoftObjectReference(Value));
	}
	else if (Next == EDcDataEntry::SoftClassReference)
	{
		FSoftClassPath Value;
		DC_TRY(Reader->ReadSoftClassReference(&Value));
		DC_TRY(Writer->WriteSoftClassReference(Value));
	}
	else if (Next == EDcDataEntry::InterfaceReference)
	{
		FScriptInterface Value;
		DC_TRY(Reader->ReadInterfaceReference(&Value));
		DC_TRY(Writer->WriteInterfaceReference(Value));
	}
	else if (Next == EDcDataEntry::FieldPath)
	{
		FFieldPath Value;
		DC_TRY(Reader->ReadFieldPath(&Value));
		DC_TRY(Writer->WriteFieldPath(Value));
	}
	else if (Next == EDcDataEntry::Delegate)
	{
		FScriptDelegate Value;
		DC_TRY(Reader->ReadDelegate(&Value));
		DC_TRY(Writer->WriteDelegate(Value));
	}
	else if (Next == EDcDataEntry::MulticastInlineDelegate)
	{
		FMulticastScriptDelegate Value;
		DC_TRY(Reader->ReadMulticastInlineDelegate(&Value));
		DC_TRY(Writer->WriteMulticastInlineDelegate(Value));
	}
	else if (Next == EDcDataEntry::MulticastSparseDelegate)
	{
		FMulticastScriptDelegate Value;
		DC_TRY(Reader->ReadMulticastSparseDelegate(&Value));
		DC_TRY(Writer->WriteMulticastSparseDelegate(Value));
	}
	else if (Next == EDcDataEntry::Int8)
	{
		int8 Value;
		DC_TRY(Reader->ReadInt8(&Value));
		DC_TRY(Writer->WriteInt8(Value));
	}
	else if (Next == EDcDataEntry::Int16)
	{
		int16 Value;
		DC_TRY(Reader->ReadInt16(&Value));
		DC_TRY(Writer->WriteInt16(Value));
	}
	else if (Next == EDcDataEntry::Int32)
	{
		int32 Value;
		DC_TRY(Reader->ReadInt32(&Value));
		DC_TRY(Writer->WriteInt32(Value));
	}
	else if (Next == EDcDataEntry::Int64)
	{
		int64 Value;
		DC_TRY(Reader->ReadInt64(&Value));
		DC_TRY(Writer->WriteInt64(Value));
	}
	else if (Next == EDcDataEntry::UInt8)
	{
		uint8 Value;
		DC_TRY(Reader->ReadUInt8(&Value));
		DC_TRY(Writer->WriteUInt8(Value));
	}
	else if (Next == EDcDataEntry::UInt16)
	{
		uint16 Value;
		DC_TRY(Reader->ReadUInt16(&Value));
		DC_TRY(Writer->WriteUInt16(Value));
	}
	else if (Next == EDcDataEntry::UInt32)
	{
		uint32 Value;
		DC_TRY(Reader->ReadUInt32(&Value));
		DC_TRY(Writer->WriteUInt32(Value));
	}
	else if (Next == EDcDataEntry::UInt64)
	{
		uint64 Value;
		DC_TRY(Reader->ReadUInt64(&Value));
		DC_TRY(Writer->WriteUInt64(Value));
	}
	else if (Next == EDcDataEntry::Float)
	{
		float Value;
		DC_TRY(Reader->ReadFloat(&Value));
		DC_TRY(Writer->WriteFloat(Value));
	}
	else if (Next == EDcDataEntry::Double)
	{
		double Value;
		DC_TRY(Reader->ReadDouble(&Value));
		DC_TRY(Writer->WriteDouble(Value));
	}
	else if (Next == EDcDataEntry::Ended)
	{
		//	need to handle Ended outside
		return DcNoEntry();
	}
	else
	{
		return DcNoEntry();
	}

	return DcOk();
}

FDcResult ExpectMetaKey(const FString& Actual, const TCHAR* Expect)
{
	return Actual == Expect
		? DcOk()
		: DC_FAIL(DcDDeserialize, MetaKeyMismatch)
			<< Expect << Actual;
}

FDcResult TryStaticFindObject(UClass* Class, UObject* Outer, const TCHAR* Name, bool ExactClass, UObject*& OutObject)
{
	check(Class);
	UObject* Ret = StaticFindObject(Class, Outer, Name, ExactClass);
	if (!Ret)
	{
		return DC_FAIL(DcDDeserialize, UObjectByStrNotFound) << Class->GetFName() << Name;
	}
	else
	{
		OutObject = Ret;
		return DcOk();
	}
}

FDcResult TryStaticLoadObject(UClass* Class, UObject* Outer, const TCHAR* LoadPath, UObject*& OutObject)
{
	check(Class);
	UObject* Ret = StaticLoadObject(Class, Outer, LoadPath, nullptr);
	if (!Ret)
	{
		return DC_FAIL(DcDDeserialize, UObjectByStrNotFound) << Class->GetFName() << LoadPath;
	}
	else
	{
		OutObject = Ret;
		return DcOk();
	}
}

FDcResult ExpectLhsChildOfRhs(UClass* Lhs, UClass* Rhs)
{
	return Lhs->IsChildOf(Rhs)
		? DcOk()
		: DC_FAIL(DcDDeserialize, ClassLhsIsNotChildOfRhs)
			<< Lhs->GetFName() << Rhs->GetFName();

}

FDcResult ExpectNonAbstract(UClass* Class)
{
	return !Class->HasAnyClassFlags(CLASS_Abstract)
		? DcOk()
		: DC_FAIL(DcDDeserialize, ClassExpectNonAbstract)
			<< Class->GetFName();
}

}	// namespace DcDeserializeUtils

