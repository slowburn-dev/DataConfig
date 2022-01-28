#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"

namespace DcSerDeUtils {

bool IsMeta(const FString& Str)
{
	return !Str.IsEmpty()
		&& Str[0] == TEXT('$');
}

FDcResult ExpectMetaKey(const FString& Actual, const TCHAR* Expect)
{
	return Actual == Expect
		? DcOk()
		: DC_FAIL(DcDSerDe, MetaKeyMismatch)
			<< Expect << Actual;
}


FDcResult DispatchPipeVisit(EDcDataEntry Next, FDcReader* Reader, FDcWriter* Writer)
{
	return DcPipe_Dispatch(Next, Reader, Writer);
}

FDcResult TryStaticFindObject(UClass* Class, UObject* Outer, const TCHAR* Name, bool ExactClass, UObject*& OutObject)
{
	check(Class);
	UObject* Ret = StaticFindObject(Class, Outer, Name, ExactClass);
	if (!Ret)
	{
		return DC_FAIL(DcDSerDe, UObjectByStrNotFound) << Class->GetFName() << Name;
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
		return DC_FAIL(DcDSerDe, UObjectByStrNotFound) << Class->GetFName() << LoadPath;
	}
	else
	{
		OutObject = Ret;
		return DcOk();
	}
}

FDcResult TryStaticLocateObject(UClass* Class, const FString& Str, UObject*& OutObject)
{
	if (Str.StartsWith(TEXT("/")))
	{
		//	/Game/Path/To/Object
		//	`Game` is a Mount Point, and there's no `.uasset` suffix
		UObject* Loaded;
		DC_TRY(DcSerDeUtils::TryStaticLoadObject(Class, nullptr, *Str, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
	else if (Str.Len() > 1
		&& Str.StartsWith(TEXT("'"))
		&& Str.EndsWith(TEXT("'")))
	{
		//	'/Foo/Bar'
		//	single quoted name, it's object name starting with '/'
		FString Unquoted = Str.Mid(1, Str.Len() - 2);
		
		UObject* Loaded;
		DC_TRY(DcSerDeUtils::TryStaticFindObject(Class, ANY_PACKAGE, *Str, false, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
	else
	{
		//	Foo
		//	try find by name
		UObject* Loaded;
		DC_TRY(DcSerDeUtils::TryStaticFindObject(Class, ANY_PACKAGE, *Str, false, Loaded));

		OutObject = Loaded;
		return DcOk();
	}
}

FDcResult ExpectLhsChildOfRhs(UClass* Lhs, UClass* Rhs)
{
	return Lhs->IsChildOf(Rhs)
		? DcOk()
		: DC_FAIL(DcDSerDe, ClassLhsIsNotChildOfRhs)
			<< Lhs->GetFName() << Rhs->GetFName();

}

FDcResult ExpectNonAbstract(UClass* Class)
{
	return !Class->HasAnyClassFlags(CLASS_Abstract)
		? DcOk()
		: DC_FAIL(DcDSerDe, ClassExpectNonAbstract)
			<< Class->GetFName();
}

FString FormatObjectName(UObject* Object)
{
	check(Object);
	if (Object->HasAnyFlags(RF_WasLoaded))
	{
		FString PathName = Object->GetPathName();
		return FString::Printf(TEXT("%s'%s'"),
			*Object->GetClass()->GetName(),
			*PathName.ReplaceQuotesWithEscapedQuotes()
		);
	}
	else
	{
		FString NameStr = Object->GetName();
		if (NameStr.StartsWith(TEXT("/")))
		{
			return FString::Printf(TEXT("'%s'"),
				*NameStr
			);
		}
		else
		{
			return NameStr;
		}
	}
}

FDcResult DispatchNoopRead(EDcDataEntry Next, FDcReader* Reader)
{
	switch (Next)
	{
		case EDcDataEntry::Nil:
			return Reader->ReadNil();
		case EDcDataEntry::Bool:
			return Reader->ReadBool(nullptr);
		case EDcDataEntry::Name:
			return Reader->ReadName(nullptr);
		case EDcDataEntry::String:
			return Reader->ReadString(nullptr);
		case EDcDataEntry::Text:
			return Reader->ReadText(nullptr);
		case EDcDataEntry::Enum:
			return Reader->ReadEnum(nullptr);
		case EDcDataEntry::Int8:
			return Reader->ReadInt8(nullptr);
		case EDcDataEntry::Int16:
			return Reader->ReadInt16(nullptr);
		case EDcDataEntry::Int32:
			return Reader->ReadInt32(nullptr);
		case EDcDataEntry::Int64:
			return Reader->ReadInt64(nullptr);
		case EDcDataEntry::UInt8:
			return Reader->ReadUInt8(nullptr);
		case EDcDataEntry::UInt16:
			return Reader->ReadUInt16(nullptr);
		case EDcDataEntry::UInt32:
			return Reader->ReadUInt32(nullptr);
		case EDcDataEntry::UInt64:
			return Reader->ReadUInt64(nullptr);
		case EDcDataEntry::Float:
			return Reader->ReadFloat(nullptr);
		case EDcDataEntry::Double:
			return Reader->ReadDouble(nullptr);
		case EDcDataEntry::Blob:
			return Reader->ReadBlob(nullptr);
		case EDcDataEntry::StructRoot:
			return Reader->ReadStructRoot();
		case EDcDataEntry::StructEnd:
			return Reader->ReadStructEnd();
		case EDcDataEntry::MapRoot:
			return Reader->ReadMapRoot();
		case EDcDataEntry::MapEnd:
			return Reader->ReadMapEnd();
		case EDcDataEntry::ClassRoot:
			return Reader->ReadClassRoot();
		case EDcDataEntry::ClassEnd:
			return Reader->ReadClassEnd();
		case EDcDataEntry::ArrayRoot:
			return Reader->ReadArrayRoot();
		case EDcDataEntry::ArrayEnd:
			return Reader->ReadArrayEnd();
		case EDcDataEntry::SetRoot:
			return Reader->ReadSetRoot();
		case EDcDataEntry::SetEnd:
			return Reader->ReadSetEnd();
		case EDcDataEntry::ObjectReference:
			return Reader->ReadObjectReference(nullptr);
		case EDcDataEntry::ClassReference:
			return Reader->ReadClassReference(nullptr);
		case EDcDataEntry::WeakObjectReference:
			return Reader->ReadWeakObjectReference(nullptr);
		case EDcDataEntry::LazyObjectReference:
			return Reader->ReadLazyObjectReference(nullptr);
		case EDcDataEntry::SoftObjectReference:
			return Reader->ReadSoftObjectReference(nullptr);
		case EDcDataEntry::SoftClassReference:
			return Reader->ReadSoftClassReference(nullptr);
		case EDcDataEntry::InterfaceReference:
			return Reader->ReadInterfaceReference(nullptr);
		case EDcDataEntry::FieldPath:
			return Reader->ReadFieldPath(nullptr);
		case EDcDataEntry::Delegate:
			return Reader->ReadDelegate(nullptr);
		case EDcDataEntry::MulticastInlineDelegate:
			return Reader->ReadMulticastInlineDelegate(nullptr);
		case EDcDataEntry::MulticastSparseDelegate:
			return Reader->ReadMulticastSparseDelegate(nullptr);
		case EDcDataEntry::Extension:
			return DC_FAIL(DcDSerDe, PipeUnhandledExtension);
		case EDcDataEntry::Ended:
			return DC_FAIL(DcDSerDe, PipeUnhandledEnded);
		default:
			return DcNoEntry();
	}
	return DcNoEntry();
}

FDcResult ReadNoopConsumeValue(FDcReader* Reader)
{
	EDcDataEntry Next;
	DC_TRY(Reader->PeekRead(&Next));

	switch(Next)
	{
		case EDcDataEntry::ArrayRoot:
		{
			int ArrayCount = 1;
			DC_TRY(Reader->ReadArrayRoot());
			while (true)
			{
				EDcDataEntry Cur;
				DC_TRY(Reader->PeekRead(&Cur));

				if (Cur == EDcDataEntry::ArrayEnd)
					--ArrayCount;
				else if (Cur == EDcDataEntry::ArrayRoot)
					++ArrayCount;

				DC_TRY(DispatchNoopRead(Cur, Reader));
				if (ArrayCount == 0)
					return DcOk();
			}
			break;
		}
		case EDcDataEntry::MapRoot:
		{
			int MapCount = 1;
			DC_TRY(Reader->ReadMapRoot());
			while (true)
			{
				EDcDataEntry Cur;
				DC_TRY(Reader->PeekRead(&Cur));

				if (Cur == EDcDataEntry::MapEnd)
					--MapCount;
				else if (Cur == EDcDataEntry::MapRoot)
					++MapCount;

				DC_TRY(DispatchNoopRead(Cur, Reader));
				if (MapCount == 0)
					return DcOk();
			}
			break;
		}
		case EDcDataEntry::SetRoot:
		{
			int SetCount = 1;
			DC_TRY(Reader->ReadSetRoot());
			while (true)
			{
				EDcDataEntry Cur;
				DC_TRY(Reader->PeekRead(&Cur));

				if (Cur == EDcDataEntry::SetEnd)
					--SetCount;
				else if (Cur == EDcDataEntry::SetRoot)
					++SetCount;

				DC_TRY(DispatchNoopRead(Cur, Reader));
				if (SetCount == 0)
					return DcOk();
			}
			break;
		}
		case EDcDataEntry::ClassRoot:
		{
			int ClassCount = 1;
			DC_TRY(Reader->ReadClassRoot());
			while (true)
			{
				EDcDataEntry Cur;
				DC_TRY(Reader->PeekRead(&Cur));

				if (Cur == EDcDataEntry::ClassEnd)
					--ClassCount;
				else if (Cur == EDcDataEntry::ClassRoot)
					++ClassCount;

				DC_TRY(DispatchNoopRead(Cur, Reader));
				if (ClassCount == 0)
					return DcOk();
			}
			break;
		}
		case EDcDataEntry::StructRoot:
		{
			int StructCount = 1;
			DC_TRY(Reader->ReadStructRoot());
			while (true)
			{
				EDcDataEntry Cur;
				DC_TRY(Reader->PeekRead(&Cur));

				if (Cur == EDcDataEntry::StructEnd)
					--StructCount;
				else if (Cur == EDcDataEntry::StructRoot)
					++StructCount;

				DC_TRY(DispatchNoopRead(Cur, Reader));
				if (StructCount == 0)
					return DcOk();
			}
			break;
		}
		default:
		{
			DC_TRY(DispatchNoopRead(Next, Reader));
			break;
		}
	}

	return DcOk();
}


}	// namespace DcSerDeUtils
