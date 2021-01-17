#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

namespace DcAutomationUtils
{

template<typename T>
static typename TEnableIf<TDcIsDataVariantCompatible<T>::Value, FDcResult>::Type
_ExpectEqual(const T& Lhs, const T& Rhs)
{
	if (Lhs != Rhs)
	{
		return DC_FAIL(DcDReadWrite, DataTypeUnequalLhsRhs)
			<< DcTypeUtils::TDcDataEntryType<T>::Value << Lhs << Rhs;
	}
	else
	{
		return DcOk();
	}
}

template<typename T>
static typename TEnableIf<!TDcIsDataVariantCompatible<T>::Value, FDcResult>::Type
_ExpectEqual(const T& Lhs, const T& Rhs)
{
	if (Lhs != Rhs)
	{
		return DC_FAIL(DcDReadWrite, DataTypeUnequal)
			<< DcTypeUtils::TDcDataEntryType<T>::Value;
	}
	else
	{
		return DcOk();
	}
}

FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum)
{
	FDcPropertyReader LhsReader(LhsDatum);
	FDcPropertyReader RhsReader(RhsDatum);

	while (true)
	{
		EDcDataEntry Next;
		EDcDataEntry RhsPeekEntry;
		DC_TRY(LhsReader.PeekRead(&Next));
		DC_TRY(RhsReader.PeekRead(&RhsPeekEntry));

		DC_TRY(_ExpectEqual(Next, RhsPeekEntry));

		if (Next == EDcDataEntry::Ended)
		{
			return DcOk();
		}
		else if (Next == EDcDataEntry::Nil)
		{
			DC_TRY(LhsReader.ReadNil());
			DC_TRY(RhsReader.ReadNil());
		}
		else if (Next == EDcDataEntry::Bool)
		{
			bool Lhs;
			DC_TRY(LhsReader.ReadBool(&Lhs));

			bool Rhs;
			DC_TRY(RhsReader.ReadBool(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Name)
		{
			FName Lhs;
			DC_TRY(LhsReader.ReadName(&Lhs));

			FName Rhs;
			DC_TRY(RhsReader.ReadName(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::String)
		{
			FString Lhs;
			DC_TRY(LhsReader.ReadString(&Lhs));

			FString Rhs;
			DC_TRY(RhsReader.ReadString(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Text)
		{
			FText Lhs;
			DC_TRY(LhsReader.ReadText(&Lhs));

			FText Rhs;
			DC_TRY(RhsReader.ReadText(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.ToString(), Rhs.ToString()));
		}
		else if (Next == EDcDataEntry::Enum)
		{
			FDcEnumData Lhs;
			DC_TRY(LhsReader.ReadEnum(&Lhs));

			FDcEnumData Rhs;
			DC_TRY(RhsReader.ReadEnum(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.Type, Rhs.Type));
			DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
			DC_TRY(_ExpectEqual(Lhs.bIsUnsigned, Rhs.bIsUnsigned));
			if (Lhs.bIsUnsigned)
				DC_TRY(_ExpectEqual(Lhs.Signed64, Rhs.Signed64));
			else
				DC_TRY(_ExpectEqual(Lhs.Unsigned64, Rhs.Unsigned64));
		}
		else if (Next == EDcDataEntry::StructRoot)
		{
			FDcStructStat Lhs;
			DC_TRY(LhsReader.ReadStructRoot(&Lhs));

			FDcStructStat Rhs;
			DC_TRY(RhsReader.ReadStructRoot(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
		}
		else if (Next == EDcDataEntry::StructEnd)
		{
			FDcStructStat Lhs;
			DC_TRY(LhsReader.ReadStructEnd(&Lhs));

			FDcStructStat Rhs;
			DC_TRY(RhsReader.ReadStructEnd(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
		}
		else if (Next == EDcDataEntry::MapRoot)
		{
			DC_TRY(LhsReader.ReadMapRoot());
			DC_TRY(RhsReader.ReadMapRoot());
		}
		else if (Next == EDcDataEntry::MapEnd)
		{
			DC_TRY(LhsReader.ReadMapEnd());
			DC_TRY(RhsReader.ReadMapEnd());
		}
		else if (Next == EDcDataEntry::ClassRoot)
		{
			FDcClassStat Lhs;
			DC_TRY(LhsReader.ReadClassRoot(&Lhs));

			FDcClassStat Rhs;
			DC_TRY(RhsReader.ReadClassRoot(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
		}
		else if (Next == EDcDataEntry::ClassEnd)
		{
			FDcClassStat Lhs;
			DC_TRY(LhsReader.ReadClassEnd(&Lhs));

			FDcClassStat Rhs;
			DC_TRY(RhsReader.ReadClassEnd(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
		}
		else if (Next == EDcDataEntry::ArrayRoot)
		{
			DC_TRY(LhsReader.ReadArrayRoot());
			DC_TRY(RhsReader.ReadArrayRoot());
		}
		else if (Next == EDcDataEntry::ArrayEnd)
		{
			DC_TRY(LhsReader.ReadArrayEnd());
			DC_TRY(RhsReader.ReadArrayEnd());
		}
		else if (Next == EDcDataEntry::SetRoot)
		{
			DC_TRY(LhsReader.ReadSetRoot());
			DC_TRY(RhsReader.ReadSetRoot());
		}
		else if (Next == EDcDataEntry::SetEnd)
		{
			DC_TRY(LhsReader.ReadSetEnd());
			DC_TRY(RhsReader.ReadSetEnd());
		}
		else if (Next == EDcDataEntry::ObjectReference)
		{
			UObject* Lhs;
			DC_TRY(LhsReader.ReadObjectReference(&Lhs));

			UObject* Rhs;
			DC_TRY(RhsReader.ReadObjectReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::ClassReference)
		{
			UClass* Lhs;
			DC_TRY(LhsReader.ReadClassReference(&Lhs));

			UClass* Rhs;
			DC_TRY(RhsReader.ReadClassReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::WeakObjectReference)
		{
			FWeakObjectPtr Lhs;
			DC_TRY(LhsReader.ReadWeakObjectReference(&Lhs));

			FWeakObjectPtr Rhs;
			DC_TRY(RhsReader.ReadWeakObjectReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::LazyObjectReference)
		{
			FLazyObjectPtr Lhs;
			DC_TRY(LhsReader.ReadLazyObjectReference(&Lhs));

			FLazyObjectPtr Rhs;
			DC_TRY(RhsReader.ReadLazyObjectReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::SoftObjectReference)
		{
			FSoftObjectPath Lhs;
			DC_TRY(LhsReader.ReadSoftObjectReference(&Lhs));

			FSoftObjectPath Rhs;
			DC_TRY(RhsReader.ReadSoftObjectReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::SoftClassReference)
		{
			FSoftClassPath Lhs;
			DC_TRY(LhsReader.ReadSoftClassReference(&Lhs));

			FSoftClassPath Rhs;
			DC_TRY(RhsReader.ReadSoftClassReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::InterfaceReference)
		{
			FScriptInterface Lhs;
			DC_TRY(LhsReader.ReadInterfaceReference(&Lhs));

			FScriptInterface Rhs;
			DC_TRY(RhsReader.ReadInterfaceReference(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::FieldPath)
		{
			FFieldPath Lhs;
			DC_TRY(LhsReader.ReadFieldPath(&Lhs));

			FFieldPath Rhs;
			DC_TRY(RhsReader.ReadFieldPath(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Delegate)
		{
			FScriptDelegate Lhs;
			DC_TRY(LhsReader.ReadDelegate(&Lhs));

			FScriptDelegate Rhs;
			DC_TRY(RhsReader.ReadDelegate(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::MulticastInlineDelegate)
		{
			FMulticastScriptDelegate Lhs;
			DC_TRY(LhsReader.ReadMulticastInlineDelegate(&Lhs));

			FMulticastScriptDelegate Rhs;
			DC_TRY(RhsReader.ReadMulticastInlineDelegate(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.ToString<UObject>(), Rhs.ToString<UObject>()));
		}
		else if (Next == EDcDataEntry::MulticastSparseDelegate)
		{
			FMulticastScriptDelegate Lhs;
			DC_TRY(LhsReader.ReadMulticastSparseDelegate(&Lhs));

			FMulticastScriptDelegate Rhs;
			DC_TRY(RhsReader.ReadMulticastSparseDelegate(&Rhs));

			DC_TRY(_ExpectEqual(Lhs.ToString<UObject>(), Rhs.ToString<UObject>()));
		}
		else if (Next == EDcDataEntry::Int8)
		{
			int8 Lhs;
			DC_TRY(LhsReader.ReadInt8(&Lhs));

			int8 Rhs;
			DC_TRY(RhsReader.ReadInt8(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Int16)
		{
			int16 Lhs;
			DC_TRY(LhsReader.ReadInt16(&Lhs));

			int16 Rhs;
			DC_TRY(RhsReader.ReadInt16(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Int32)
		{
			int32 Lhs;
			DC_TRY(LhsReader.ReadInt32(&Lhs));

			int32 Rhs;
			DC_TRY(RhsReader.ReadInt32(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Int64)
		{
			int64 Lhs;
			DC_TRY(LhsReader.ReadInt64(&Lhs));

			int64 Rhs;
			DC_TRY(RhsReader.ReadInt64(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::UInt8)
		{
			uint8 Lhs;
			DC_TRY(LhsReader.ReadUInt8(&Lhs));

			uint8 Rhs;
			DC_TRY(RhsReader.ReadUInt8(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::UInt16)
		{
			uint16 Lhs;
			DC_TRY(LhsReader.ReadUInt16(&Lhs));

			uint16 Rhs;
			DC_TRY(RhsReader.ReadUInt16(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::UInt32)
		{
			uint32 Lhs;
			DC_TRY(LhsReader.ReadUInt32(&Lhs));

			uint32 Rhs;
			DC_TRY(RhsReader.ReadUInt32(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::UInt64)
		{
			uint64 Lhs;
			DC_TRY(LhsReader.ReadUInt64(&Lhs));

			uint64 Rhs;
			DC_TRY(RhsReader.ReadUInt64(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Float)
		{
			float Lhs;
			DC_TRY(LhsReader.ReadFloat(&Lhs));

			float Rhs;
			DC_TRY(RhsReader.ReadFloat(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else if (Next == EDcDataEntry::Double)
		{
			double Lhs;
			DC_TRY(LhsReader.ReadDouble(&Lhs));

			double Rhs;
			DC_TRY(RhsReader.ReadDouble(&Rhs));

			DC_TRY(_ExpectEqual(Lhs, Rhs));
		}
		else
		{
			return DcNoEntry();
		}
	}

	return DcOk();
}

void DumpToLog(FDcPropertyDatum Datum)
{
	FOutputDevice* WarnOut = (FOutputDevice*)GWarn;
	if (Datum.IsNone())
	{
		return WarnOut->Log(TEXT("# Datum: <None>"));
	}
	else
	{
		FDcScopedEnv ScopedEnv{};
		ScopedEnv.Get().DiagConsumer = MakeShareable(new FDcDefaultLogDiagnosticConsumer());

		WarnOut->Log(TEXT("-----------------------------------------"));
		WarnOut->Logf(TEXT("# Datum: '%s', '%s'"), *Datum.Property.GetClassName(), *Datum.Property.GetName());

		FDcPropertyReader PropReader(Datum);
		FDcPrettyPrintWriter PrettyWriter(*(FOutputDevice*)GWarn);
		FDcPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);

		if (!PrettyPrintVisit.PipeVisit().Ok())
			ScopedEnv.Get().FlushDiags();
		WarnOut->Log(TEXT("-----------------------------------------"));
	}
}

FString DumpFormat(FDcPropertyDatum Datum)
{
	struct FStrCollectDevice : public FString, public FOutputDevice
	{
		FStrCollectDevice() : FString()
		{}

		void Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const class FName& Category) override
		{
			FString::operator+=((TCHAR*)InData);
			*this += LINE_TERMINATOR;
		}

		virtual FString& operator+=(const FString& Other)
		{
			return FString::operator+=(Other);
		}
	};

	if (Datum.IsNone())
	{
		return TEXT("# Datum: <None>");
	}
	else
	{
		FDcScopedEnv ScopedEnv{};
		FStrCollectDevice StrOutput;
		ScopedEnv.Get().DiagConsumer = MakeShareable(new FDcStringDiagnosticConsumer{&StrOutput});

		StrOutput.Log(TEXT("-----------------------------------------"));
		StrOutput.Logf(TEXT("# Datum: '%s', '%s'"), *Datum.Property.GetClassName(), *Datum.Property.GetName());

		FDcPropertyReader PropReader(Datum);
		FDcPrettyPrintWriter PrettyWriter(StrOutput);
		FDcPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);

		if (!PrettyPrintVisit.PipeVisit().Ok())
			ScopedEnv.Get().FlushDiags();

		StrOutput.Log(TEXT("-----------------------------------------"));

		return MoveTemp(StrOutput);
	}
}

//	UXXX(meta=(Foo)) only get compiled in `WITH_EDITOR`
//	need to manually amend it on Program targets
void AmendMetaData(UField* Field, const FName& MetaKey, const TCHAR* MetaValue)
{
	check(Field);
	if (!Field->HasMetaData(MetaKey))
		Field->SetMetaData(MetaKey, MetaValue);
}

void AmendMetaData(UStruct* Struct, const FName& FieldName, const FName& MetaKey, const TCHAR* MetaValue)
{
	check(Struct);
	FProperty* Property = DcPropertyUtils::NextPropertyByName(Struct->PropertyLink, FieldName);
	check(Property);
	if (!Property->HasMetaData(MetaKey))
		Property->SetMetaData(MetaKey, MetaValue);
}

}	// namespace DcAutomationUtils

#if DC_BUILD_DEBUG
void FDcDebug::DumpStruct(char* StructNameChars, void* Ptr)
{
	FString StructNameStr(StructNameChars);
	UScriptStruct* LoadStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *StructNameStr, true);
	if (LoadStruct == nullptr)
	{
		FPlatformMisc::LowLevelOutputDebugString(*FString::Printf(TEXT("- DcDebug DumpStruct: Failed to find struct '%s'"), *StructNameStr));
		return;
	}

	FString Dumped = DcAutomationUtils::DumpFormat(FDcPropertyDatum(LoadStruct, Ptr));
	FPlatformMisc::LowLevelOutputDebugString(*Dumped);
}

void FDcDebug::DumpObject(UObject* Obj)
{
	FString Dumped = DcAutomationUtils::DumpFormat(FDcPropertyDatum(Obj));
	FPlatformMisc::LowLevelOutputDebugString(*Dumped);
}

void FDcDebug::DumpDatum(const FDcPropertyDatum& Datum)
{
	FString Dumped = DcAutomationUtils::DumpFormat(Datum);
	FPlatformMisc::LowLevelOutputDebugString(*Dumped);
}

FDcDebug gDcDebug;
#endif // DC_BUILD_DEBUG

