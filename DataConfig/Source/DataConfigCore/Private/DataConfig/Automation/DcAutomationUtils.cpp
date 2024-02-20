#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/DcCorePrivate.h"
#include "DataConfig/Source/DcSourceUtils.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"

namespace DcAutomationUtils
{

template<typename T>
static typename TEnableIf<DcTypeUtils::TIsDataEntryType<T>::Value, FDcResult>::Type
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
static typename TEnableIf<!DcTypeUtils::TIsDataEntryType<T>::Value, FDcResult>::Type
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

template<>
FDcResult _ExpectEqual<EDcDataEntry>(const EDcDataEntry& Lhs, const EDcDataEntry& Rhs)
{
	if (Lhs != Rhs)
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< Lhs << Rhs;
	}
	else
	{
		return DcOk();
	}
}

FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum, EReadDatumEqualType Type)
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

		switch (Next)
		{
			case EDcDataEntry::Ended:
			{
				return DcOk();
			}
			case EDcDataEntry::None:
			{
				DC_TRY(LhsReader.ReadNone());
				DC_TRY(RhsReader.ReadNone());
				break;
			}
			case EDcDataEntry::Bool:
			{
				bool Lhs;
				DC_TRY(LhsReader.ReadBool(&Lhs));

				bool Rhs;
				DC_TRY(RhsReader.ReadBool(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Name:
			{
				FName Lhs;
				DC_TRY(LhsReader.ReadName(&Lhs));

				FName Rhs;
				DC_TRY(RhsReader.ReadName(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::String:
			{
				FString Lhs;
				DC_TRY(LhsReader.ReadString(&Lhs));

				FString Rhs;
				DC_TRY(RhsReader.ReadString(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Text:
			{
				FText Lhs;
				DC_TRY(LhsReader.ReadText(&Lhs));

				FText Rhs;
				DC_TRY(RhsReader.ReadText(&Rhs));

				DC_TRY(_ExpectEqual(Lhs.ToString(), Rhs.ToString()));
				break;
			}
			case EDcDataEntry::Enum:
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
				break;
			}
			case EDcDataEntry::StructRoot:
			{
				FDcStructAccess Lhs;
				DC_TRY(LhsReader.ReadStructRootAccess(Lhs));

				FDcStructAccess Rhs;
				DC_TRY(RhsReader.ReadStructRootAccess(Rhs));

				DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
				break;
			}
			case EDcDataEntry::StructEnd:
			{
				FDcStructAccess Lhs;
				DC_TRY(LhsReader.ReadStructEndAccess(Lhs));

				FDcStructAccess Rhs;
				DC_TRY(RhsReader.ReadStructEndAccess(Rhs));

				DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
				break;
			}
			case EDcDataEntry::MapRoot:
			{
				DC_TRY(LhsReader.ReadMapRoot());
				DC_TRY(RhsReader.ReadMapRoot());
				break;
			}
			case EDcDataEntry::MapEnd:
			{
				DC_TRY(LhsReader.ReadMapEnd());
				DC_TRY(RhsReader.ReadMapEnd());
				break;
			}
			case EDcDataEntry::ClassRoot:
			{
				FDcClassAccess Lhs;
				if (Type == EReadDatumEqualType::ExpandAllObjects)
					Lhs.Control = FDcClassAccess::EControl::ExpandObject;
				DC_TRY(LhsReader.ReadClassRootAccess(Lhs));

				FDcClassAccess Rhs;
				if (Type == EReadDatumEqualType::ExpandAllObjects)
					Rhs.Control = FDcClassAccess::EControl::ExpandObject;
				DC_TRY(RhsReader.ReadClassRootAccess(Rhs));

				DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
				break;
			}
			case EDcDataEntry::ClassEnd:
			{
				FDcClassAccess Lhs;
				if (Type == EReadDatumEqualType::ExpandAllObjects)
					Lhs.Control = FDcClassAccess::EControl::ExpandObject;
				DC_TRY(LhsReader.ReadClassEndAccess(Lhs));

				FDcClassAccess Rhs;
				if (Type == EReadDatumEqualType::ExpandAllObjects)
					Lhs.Control = FDcClassAccess::EControl::ExpandObject;
				DC_TRY(RhsReader.ReadClassEndAccess(Rhs));

				DC_TRY(_ExpectEqual(Lhs.Name, Rhs.Name));
				break;
			}
			case EDcDataEntry::ArrayRoot:
			{
				DC_TRY(LhsReader.ReadArrayRoot());
				DC_TRY(RhsReader.ReadArrayRoot());
				break;
			}
			case EDcDataEntry::ArrayEnd:
			{
				DC_TRY(LhsReader.ReadArrayEnd());
				DC_TRY(RhsReader.ReadArrayEnd());
				break;
			}
			case EDcDataEntry::SetRoot:
			{
				DC_TRY(LhsReader.ReadSetRoot());
				DC_TRY(RhsReader.ReadSetRoot());
				break;
			}
			case EDcDataEntry::SetEnd:
			{
				DC_TRY(LhsReader.ReadSetEnd());
				DC_TRY(RhsReader.ReadSetEnd());
				break;
			}
			case EDcDataEntry::OptionalRoot:
			{
				DC_TRY(LhsReader.ReadOptionalRoot());
				DC_TRY(RhsReader.ReadOptionalRoot());
				break;
			}
			case EDcDataEntry::OptionalEnd:
			{
				DC_TRY(LhsReader.ReadOptionalEnd());
				DC_TRY(RhsReader.ReadOptionalEnd());
				break;
			}
			case EDcDataEntry::ObjectReference:
			{
				UObject* Lhs;
				DC_TRY(LhsReader.ReadObjectReference(&Lhs));

				UObject* Rhs;
				DC_TRY(RhsReader.ReadObjectReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::ClassReference:
			{
				UClass* Lhs;
				DC_TRY(LhsReader.ReadClassReference(&Lhs));

				UClass* Rhs;
				DC_TRY(RhsReader.ReadClassReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::WeakObjectReference:
			{
				FWeakObjectPtr Lhs;
				DC_TRY(LhsReader.ReadWeakObjectReference(&Lhs));

				FWeakObjectPtr Rhs;
				DC_TRY(RhsReader.ReadWeakObjectReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::LazyObjectReference:
			{
				FLazyObjectPtr Lhs;
				DC_TRY(LhsReader.ReadLazyObjectReference(&Lhs));

				FLazyObjectPtr Rhs;
				DC_TRY(RhsReader.ReadLazyObjectReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::SoftObjectReference:
			{
				FSoftObjectPtr Lhs;
				DC_TRY(LhsReader.ReadSoftObjectReference(&Lhs));

				FSoftObjectPtr Rhs;
				DC_TRY(RhsReader.ReadSoftObjectReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::SoftClassReference:
			{
				FSoftObjectPtr Lhs;
				DC_TRY(LhsReader.ReadSoftClassReference(&Lhs));

				FSoftObjectPtr Rhs;
				DC_TRY(RhsReader.ReadSoftClassReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::InterfaceReference:
			{
				FScriptInterface Lhs;
				DC_TRY(LhsReader.ReadInterfaceReference(&Lhs));

				FScriptInterface Rhs;
				DC_TRY(RhsReader.ReadInterfaceReference(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::FieldPath:
			{
				FFieldPath Lhs;
				DC_TRY(LhsReader.ReadFieldPath(&Lhs));

				FFieldPath Rhs;
				DC_TRY(RhsReader.ReadFieldPath(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Delegate:
			{
				FScriptDelegate Lhs;
				DC_TRY(LhsReader.ReadDelegate(&Lhs));

				FScriptDelegate Rhs;
				DC_TRY(RhsReader.ReadDelegate(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::MulticastInlineDelegate:
			{
				FMulticastScriptDelegate Lhs;
				DC_TRY(LhsReader.ReadMulticastInlineDelegate(&Lhs));

				FMulticastScriptDelegate Rhs;
				DC_TRY(RhsReader.ReadMulticastInlineDelegate(&Rhs));

				DC_TRY(_ExpectEqual(Lhs.ToString<UObject>(), Rhs.ToString<UObject>()));
				break;
			}
			case EDcDataEntry::MulticastSparseDelegate:
			{
				FMulticastScriptDelegate Lhs;
				DC_TRY(LhsReader.ReadMulticastSparseDelegate(&Lhs));

				FMulticastScriptDelegate Rhs;
				DC_TRY(RhsReader.ReadMulticastSparseDelegate(&Rhs));

				DC_TRY(_ExpectEqual(Lhs.ToString<UObject>(), Rhs.ToString<UObject>()));
				break;
			}
			case EDcDataEntry::Int8:
			{
				int8 Lhs;
				DC_TRY(LhsReader.ReadInt8(&Lhs));

				int8 Rhs;
				DC_TRY(RhsReader.ReadInt8(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Int16:
			{
				int16 Lhs;
				DC_TRY(LhsReader.ReadInt16(&Lhs));

				int16 Rhs;
				DC_TRY(RhsReader.ReadInt16(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Int32:
			{
				int32 Lhs;
				DC_TRY(LhsReader.ReadInt32(&Lhs));

				int32 Rhs;
				DC_TRY(RhsReader.ReadInt32(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Int64:
			{
				int64 Lhs;
				DC_TRY(LhsReader.ReadInt64(&Lhs));

				int64 Rhs;
				DC_TRY(RhsReader.ReadInt64(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::UInt8:
			{
				uint8 Lhs;
				DC_TRY(LhsReader.ReadUInt8(&Lhs));

				uint8 Rhs;
				DC_TRY(RhsReader.ReadUInt8(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::UInt16:
			{
				uint16 Lhs;
				DC_TRY(LhsReader.ReadUInt16(&Lhs));

				uint16 Rhs;
				DC_TRY(RhsReader.ReadUInt16(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::UInt32:
			{
				uint32 Lhs;
				DC_TRY(LhsReader.ReadUInt32(&Lhs));

				uint32 Rhs;
				DC_TRY(RhsReader.ReadUInt32(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::UInt64:
			{
				uint64 Lhs;
				DC_TRY(LhsReader.ReadUInt64(&Lhs));

				uint64 Rhs;
				DC_TRY(RhsReader.ReadUInt64(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Float:
			{
				float Lhs;
				DC_TRY(LhsReader.ReadFloat(&Lhs));

				float Rhs;
				DC_TRY(RhsReader.ReadFloat(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			case EDcDataEntry::Double:
			{
				double Lhs;
				DC_TRY(LhsReader.ReadDouble(&Lhs));

				double Rhs;
				DC_TRY(RhsReader.ReadDouble(&Rhs));

				DC_TRY(_ExpectEqual(Lhs, Rhs));
				break;
			}
			default:
				return DcNoEntry();
		}
	}

	return DcOk();
}

FDcResult DumpNextNumericAsString(FDcReader* Reader, FString* OutStr)
{
	EDcDataEntry Next;
	DC_TRY(Reader->PeekRead(&Next));

	auto _WriteInt64 = [](const int64& Value)
	{
		return FString::Printf(TEXT("%lld"), Value);
	};

	auto _WriteUInt64 = [](const uint64& Value)
	{
		return FString::Printf(TEXT("%llu"), Value);
	};

	if (Next == EDcDataEntry::UInt8)
	{
		uint8 Value;
		DC_TRY(Reader->ReadUInt8(&Value));
		return ReadOutOk(OutStr, _WriteUInt64(Value));
	}
	else if (Next == EDcDataEntry::UInt16)
	{
		uint16 Value;
		DC_TRY(Reader->ReadUInt16(&Value));
		return ReadOutOk(OutStr, _WriteUInt64(Value));
	}
	else if (Next == EDcDataEntry::UInt32)
	{
		uint32 Value;
		DC_TRY(Reader->ReadUInt32(&Value));
		return ReadOutOk(OutStr, _WriteUInt64(Value));
	}
	else if (Next == EDcDataEntry::UInt64)
	{
		uint64 Value;
		DC_TRY(Reader->ReadUInt64(&Value));
		return ReadOutOk(OutStr, _WriteUInt64(Value));
	}
	else if (Next == EDcDataEntry::Int8)
	{
		int8 Value;
		DC_TRY(Reader->ReadInt8(&Value));
		return ReadOutOk(OutStr, _WriteInt64(Value));
	}
	else if (Next == EDcDataEntry::Int16)
	{
		int16 Value;
		DC_TRY(Reader->ReadInt16(&Value));
		return ReadOutOk(OutStr, _WriteInt64(Value));
	}
	else if (Next == EDcDataEntry::Int32)
	{
		int32 Value;
		DC_TRY(Reader->ReadInt32(&Value));
		return ReadOutOk(OutStr, _WriteInt64(Value));
	}
	else if (Next == EDcDataEntry::Int64)
	{
		int64 Value;
		DC_TRY(Reader->ReadInt64(&Value));
		return ReadOutOk(OutStr, _WriteInt64(Value));
	}
	else if (Next == EDcDataEntry::Float)
	{
		float Value;
		DC_TRY(Reader->ReadFloat(&Value));
		return ReadOutOk(OutStr, FString::Printf(TEXT("%.17g"), Value));
	}
	else if (Next == EDcDataEntry::Double)
	{
		double Value;
		DC_TRY(Reader->ReadDouble(&Value));
		return ReadOutOk(OutStr, FString::Printf(TEXT("%.17g"), Value));
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatchNoExpect) << Next;
	}
}
namespace DcAutomationUtilDetails
{

void DumpToOutputDevice(FDcPropertyDatum Datum, FOutputDevice& Output, TSharedPtr<IDcDiagnosticConsumer> NewEnvConsumer)
{
	if (Datum.IsNone())
	{
		return Output.Log(TEXT("# Datum: <None>"));
	}
	else
	{
		FDcScopedEnv ScopedEnv{};
		ScopedEnv.Get().DiagConsumer = NewEnvConsumer;

		Output.Log(TEXT("-----------------------------------------"));
		Output.Logf(TEXT("# Datum: '%s', '%s'"), *Datum.Property.GetClassName(), *Datum.Property.GetName());

		FDcPropertyReader PropReader(Datum);
		FDcPrettyPrintWriter PrettyWriter(Output);
		FDcPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);

		if (!PrettyPrintVisit.PipeVisit().Ok())
			ScopedEnv.Get().FlushDiags();
		Output.Log(TEXT("-----------------------------------------"));
	}
}

void DumpToOutputDevice(FDcReader* Reader, FOutputDevice& Output, TSharedPtr<IDcDiagnosticConsumer> NewEnvConsumer)
{
	check(Reader);
	FDcScopedEnv ScopedEnv{};
	ScopedEnv.Get().DiagConsumer = NewEnvConsumer;

	Output.Log(TEXT("-----------------------------------------"));
	Output.Logf(TEXT("# Reader: '%s'"), *Reader->GetId().ToString());

	FDcPrettyPrintWriter PrettyWriter(Output);
	FDcPipeVisitor PrettyPrintVisit(Reader, &PrettyWriter);

	if (!PrettyPrintVisit.PipeVisit().Ok())
		ScopedEnv.Get().FlushDiags();
	Output.Log(TEXT("-----------------------------------------"));
}

struct FLowLevelOutputDevice : public FOutputDevice
{
	void Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		FPlatformMisc::LowLevelOutputDebugString(InData);
		FPlatformMisc::LowLevelOutputDebugString(LINE_TERMINATOR);
	}
};


} // namespace DcAutomationUtilDetails



void DumpToLog(FDcPropertyDatum Datum)
{
	FOutputDevice& WarnOut = (FOutputDevice&)*GWarn;
	DcAutomationUtilDetails::DumpToOutputDevice(Datum, WarnOut, DcEnv().DiagConsumer);
}

void DumpToLowLevelDebugOutput(FDcPropertyDatum Datum)
{
	DcAutomationUtilDetails::FLowLevelOutputDevice Output;
	DcAutomationUtilDetails::DumpToOutputDevice(Datum, Output, MakeShareable(new FDcOutputDeviceDiagnosticConsumer{Output}));
}

FString DumpFormat(FDcPropertyDatum Datum)
{
	DcCorePrivate::FStringNewlineDevice Output;
	DcAutomationUtilDetails::DumpToOutputDevice(Datum, Output, MakeShareable(new FDcOutputDeviceDiagnosticConsumer{Output}));

	return MoveTemp(Output);
}

void DumpToLog(FDcReader* Reader)
{
	FOutputDevice& WarnOut = (FOutputDevice&)*GWarn;
	DcAutomationUtilDetails::DumpToOutputDevice(Reader, WarnOut, DcEnv().DiagConsumer);
}

void DumpToLowLevelDebugOutput(FDcReader* Reader)
{
	DcAutomationUtilDetails::FLowLevelOutputDevice Output;
	DcAutomationUtilDetails::DumpToOutputDevice(Reader, Output, MakeShareable(new FDcOutputDeviceDiagnosticConsumer{Output}));
}

FString DumpFormat(FDcReader* Reader)
{
	DcCorePrivate::FStringNewlineDevice Output;
	DcAutomationUtilDetails::DumpToOutputDevice(Reader, Output, MakeShareable(new FDcOutputDeviceDiagnosticConsumer{Output}));

	return MoveTemp(Output);
}

#if WITH_EDITORONLY_DATA
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
	FProperty* Property = DcPropertyUtils::FindEffectivePropertyByName(Struct, FieldName);
	check(Property);
	if (!Property->HasMetaData(MetaKey))
		Property->SetMetaData(MetaKey, MetaValue);
}
#endif // WITH_EDITORONLY_DATA


FDcPropertyDatum TryGetMemberDatum(const FDcPropertyDatum& Datum, const FName& Name)
{
	FDcPropertyDatum Ret;
	UStruct* Struct = DcPropertyUtils::TryGetStruct(Datum);
	if (!Struct)
		return Ret;

	FProperty* Property = PropertyAccessUtil::FindPropertyByName(Name, Struct);
	if (!Property)
		return Ret;

	Ret.Property = Property;
	Ret.DataPtr = Property->ContainerPtrToValuePtr<void*>(Datum.DataPtr);
	return Ret;
}

int DebugGetEnumPropertyIndex(const FDcPropertyDatum& Datum, const FName& Name)
{
	UStruct* Struct = DcPropertyUtils::TryGetStruct(Datum);
	if (!Struct)
		return INDEX_NONE;

	FFieldVariant EnumProperty = PropertyAccessUtil::FindPropertyByName(Name, Struct);
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(EnumProperty, Enum, UnderlyingProperty);
	if (!bIsEnum	
		|| Enum == nullptr
		|| UnderlyingProperty == nullptr)
		return INDEX_NONE;

	void* FieldPtr = UnderlyingProperty->ContainerPtrToValuePtr<void*>(Datum.DataPtr);
	int64 Value = UnderlyingProperty->GetSignedIntPropertyValue(FieldPtr);
	return Enum->GetIndexByValue(Value);
}

double DebugGetRealPropertyValue(const FDcPropertyDatum& Datum, const FName& Name)
{
	union {
		uint64 _U64Nan;
		double _DoubleNan;
	} _NaNUnino;
	_NaNUnino._U64Nan = 0x7ff8000000000000ull;
	const double NaNRet = _NaNUnino._DoubleNan;
	check(FMath::IsNaN(NaNRet));

	UStruct* Struct = DcPropertyUtils::TryGetStruct(Datum);
	if (!Struct)
		return NaNRet;

	FProperty* Property = PropertyAccessUtil::FindPropertyByName(Name, Struct);
	if (!Property)
		return NaNRet; 

	if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
		return FloatProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void*>(Datum.DataPtr));
	else if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(Property))
		return DoubleProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void*>(Datum.DataPtr));
	else
		return NaNRet;
}

FDcResult DeserializeFrom(FDcReader* Reader, FDcPropertyDatum Datum,
                          TFunctionRef<void(FDcDeserializeContext&)> Func, EDefaultSetupType SetupType)
{
	FDcDeserializer Deserializer;
	if (SetupType == EDefaultSetupType::SetupJSONHandlers)
	{
		DcSetupJsonDeserializeHandlers(Deserializer);
	}
	else if (SetupType == EDefaultSetupType::SetupNothing)
	{
		//	pass
	}

	FDcPropertyWriter Writer(Datum);
	FDcDeserializeContext Ctx;
	Ctx.Reader = Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Add(Datum.Property);
	Func(Ctx);
	DC_TRY(Ctx.Prepare());

	return Deserializer.Deserialize(Ctx);
}

FDcResult SerializeInto(FDcWriter* Writer, FDcPropertyDatum Datum,
	TFunctionRef<void(FDcSerializeContext&)> Func, EDefaultSetupType SetupType)
{
	FDcSerializer Serializer;
	if (SetupType == EDefaultSetupType::SetupJSONHandlers)
	{
		DcSetupJsonSerializeHandlers(Serializer);
	}
	else if (SetupType == EDefaultSetupType::SetupNothing)
	{
		//	pass
	}

	FDcPropertyReader Reader(Datum);
	FDcSerializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = Writer;
	Ctx.Serializer = &Serializer;
	Ctx.Properties.Add(Datum.Property);
	Func(Ctx);
	DC_TRY(Ctx.Prepare());

	return Serializer.Serialize(Ctx);
}

//	Trim and reindent a string literal to the first non empty indent level
FString DcReindentStringLiteral(FString Str, FString* Prefix)
{
	TArray<FString> Lines;
	Str.ParseIntoArrayLines(Lines);

	int MinIndent = TNumericLimits<int>::Max();

	auto _IsWhitespaceLine = [](const FString& Str)
	{
		for (TCHAR Ch : Str)
		{
			if (!FChar::IsWhitespace(Ch))
				return false;
		}

		return true;
	};

	int LineCount = Lines.Num();
	int FirstNonEmptyIx = 0;
	int LastNonEmptyIx = LineCount;
	{
		for (int Ix = 0; Ix < LineCount; Ix++)
		{
			if (!_IsWhitespaceLine(Lines[Ix]))
			{
				FirstNonEmptyIx = Ix;
				break;
			}
		}

		for (int Ix = 0; Ix < LineCount; Ix++)
		{
			if (!_IsWhitespaceLine(Lines[LineCount- Ix - 1]))
			{
				LastNonEmptyIx = LineCount-Ix;
				break;
			}
		}
	}

	TDcCSourceUtils<TCHAR>::StringBuilder Sb;
	{
		for (int Ix = FirstNonEmptyIx; Ix < LastNonEmptyIx; Ix++)
		{
			FString& Line = Lines[Ix];

			Line.ConvertTabsToSpacesInline(4);
			Line.TrimEndInline();
			int Spaces = 0;
			for (TCHAR Char : Line)
			{
				if (Char == ' ')
					Spaces++;
				else
					break;
			}

			if (Spaces < MinIndent)
				MinIndent = Spaces;
		}

		for (int Ix = FirstNonEmptyIx; Ix < LastNonEmptyIx; Ix++)
		{
			FString& Line = Lines[Ix];

			Line.RightChopInline(MinIndent);
			if (Prefix) Sb.Append(*Prefix);
			Sb.Append(Line);
			Sb << TCHAR('\n');
		}
	}

	return Sb.ToString();
}

}	// namespace DcAutomationUtils

#if DC_BUILD_DEBUG
void FDcDebug::DumpStruct(char* StructNameChars, void* Ptr)
{
	FString StructNameStr(StructNameChars);
	UScriptStruct* LoadStruct = DcSerDeUtils::FindFirstObject<UScriptStruct>(*StructNameStr, true);
	if (LoadStruct == nullptr)
	{
		FPlatformMisc::LowLevelOutputDebugString(*FString::Printf(TEXT("- DcDebug DumpStruct: Failed to find struct '%s'"), *StructNameStr));
		return;
	}

	DcAutomationUtils::DumpToLowLevelDebugOutput(FDcPropertyDatum(LoadStruct, Ptr));
}

void FDcDebug::DumpObject(UObject* Obj)
{
	DcAutomationUtils::DumpToLowLevelDebugOutput(FDcPropertyDatum(Obj));
}

void FDcDebug::DumpDatum(void* DatumPtr)
{
	DcAutomationUtils::DumpToLowLevelDebugOutput(*(FDcPropertyDatum*)DatumPtr);
}

FString FDcDebug::DumpName(FName* NamePtr)
{
	FString Str = NamePtr->ToString();
	FPlatformMisc::LowLevelOutputDebugString(*Str);
	FPlatformMisc::LowLevelOutputDebugString(LINE_TERMINATOR);
	return Str;
}

FDcDebug gDcDebug;
#endif // DC_BUILD_DEBUG

