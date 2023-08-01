#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackTransientDeserializers.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/SerDe/DcSerDeCommon.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"


namespace DcMsgPackHandlersDetails
{

static FORCEINLINE_DEBUGGABLE FDcResult ReadPointerRaw(FDcReader* Reader, void*& OutPtr)
{
#if PLATFORM_64BITS
	uint64 Value;
	DC_TRY(Reader->ReadUInt64(&Value));
	OutPtr = reinterpret_cast<void*>(Value);
#else
	uint32 Value;
	DC_TRY(Reader->ReadUInt32(&Value));
	OutPtr = reinterpret_cast<void*>(Value);
#endif
	return DcOk();
}

static FORCEINLINE_DEBUGGABLE FDcResult ReadPointerOrNil(FDcReader* Reader, void*& OutPtr)
{
	EDcDataEntry Next;
	DC_TRY(Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Reader->ReadNil());
		OutPtr = nullptr;
	}
#if PLATFORM_64BITS
	else if (Next == EDcDataEntry::UInt64)
	{
		DC_TRY(ReadPointerRaw(Reader, OutPtr));
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::Nil << EDcDataEntry::UInt64 << Next;
	}
#else
	else if (Next == EDcDataEntry::UInt32)
	{
		DC_TRY(ReadPointerRaw(Reader, OutPtr));
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::Nil << EDcDataEntry::UInt32 << Next;
	}
#endif

	return DcOk();
}

static FORCEINLINE_DEBUGGABLE FDcResult ReadTransientName(FDcReader* Reader, FName& OutName)
{
	uint32 ComparisonIndex;
	uint32 DisplayIndex;
	int32 Number;

	DC_TRY(Reader->ReadArrayRoot());
	DC_TRY(Reader->ReadUInt32(&ComparisonIndex));
	DC_TRY(Reader->ReadUInt32(&DisplayIndex));
	DC_TRY(Reader->ReadInt32(&Number));
	DC_TRY(Reader->ReadArrayEnd());

	FName Name(
		FNameEntryId::FromUnstableInt(ComparisonIndex),
		FNameEntryId::FromUnstableInt(DisplayIndex),
		Number
	);

	OutName = Name;
	return DcOk();
}

static FORCEINLINE_DEBUGGABLE FDcResult ReadTransientWeakObjectPtr(FDcReader* Reader, DcSerDeCommon::FWeakObjectPtrAccess& ValueAccess)
{
	DC_TRY(Reader->ReadArrayRoot());
	DC_TRY(Reader->ReadInt32(&ValueAccess.ObjectIndex));
	DC_TRY(Reader->ReadInt32(&ValueAccess.ObjectSerialNumber));
	DC_TRY(Reader->ReadArrayEnd());
	return DcOk();
}

template<typename TDelegate>
static FORCEINLINE_DEBUGGABLE FDcResult ReadTransientScriptDelegate(FDcReader* Reader, DcSerDeCommon::TDelegateAccess<TDelegate>& ValueAccess)
{
	using DcSerDeCommon::FWeakObjectPtrAccess;
	FWeakObjectPtrAccess& WeakAccess = (FWeakObjectPtrAccess&)ValueAccess.GetObject();

	DC_TRY(Reader->ReadArrayRoot());
	DC_TRY(Reader->ReadInt32(&WeakAccess.ObjectIndex));
	DC_TRY(Reader->ReadInt32(&WeakAccess.ObjectSerialNumber));
	DC_TRY(ReadTransientName(Reader, ValueAccess.GetFunctionName()));
	DC_TRY(Reader->ReadArrayEnd());

	return DcOk();
}

static FORCEINLINE_DEBUGGABLE FDcResult ReadTransientMulticastScriptDelegate(FDcReader* Reader, DcSerDeCommon::FMulticastScriptDelegateAccess& ValueAccess)
{
	using DcSerDeCommon::FScriptDelegateAccess;
	using FInvocationAccess = DcSerDeCommon::FMulticastScriptDelegateAccess::TypeInvocationAccess;
	DC_TRY(Reader->ReadArrayRoot());

	while(true)
	{
		EDcDataEntry Next;
		DC_TRY(Reader->PeekRead(&Next));
		if (Next == EDcDataEntry::ArrayEnd)
			break;

		FInvocationAccess& DelgateAccess = (FInvocationAccess&)ValueAccess.GetInvocationList().AddDefaulted_GetRef();
		DC_TRY(ReadTransientScriptDelegate(Reader, DelgateAccess));
	}

	DC_TRY(Reader->ReadArrayEnd());
	return DcOk();
}

} // namespace DcMsgPackHandlersDetails


namespace DcMsgPackHandlers
{

FDcResult HandlerTransientNameDeserialize(FDcDeserializeContext& Ctx)
{
	FName Value;
	DC_TRY(DcMsgPackHandlersDetails::ReadTransientName(Ctx.Reader, Value));
	DC_TRY(Ctx.Writer->WriteName(Value));

	return DcOk();
}

FDcResult HandlerTransientTextDeserialize(FDcDeserializeContext& Ctx)
{
	using namespace DcSerDeCommon;

	void* ObjectPtr;
	void* SharedRefPtr;
	uint32 Flags;

	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, ObjectPtr));
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, SharedRefPtr));
	DC_TRY(Ctx.Reader->ReadUInt32(&Flags));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	FText Text;
	FTextAccess& TextAccess = (FTextAccess&)Text;

#if ENGINE_MAJOR_VERSION == 5
	{
		using RefControllerType = SharedPointerInternals::TReferenceControllerBase<ESPMode::ThreadSafe>;
		RefControllerType* SharedController = (RefControllerType*)SharedRefPtr;
		SharedController->SharedReferenceCount++;

		TextAccess.TextData = UE::Core::Private::MakeSharedRef<ITextData, ESPMode::ThreadSafe>(
			(ITextData*)ObjectPtr,
			SharedController
		);
	}
#else
	{
		SharedPointerInternals::FReferenceControllerBase* SharedController = (SharedPointerInternals::FReferenceControllerBase*)SharedRefPtr;
		//	add a reference count here as we're writing it into a new slot
		SharedController->SharedReferenceCount++;

		TextAccess.TextData = UE4SharedPointer_Private::MakeSharedRef<ITextData, ESPMode::ThreadSafe>(
			(ITextData*)ObjectPtr,
			SharedController
		);
	}
#endif

	TextAccess.Flags = Flags;
	DC_TRY(Ctx.Writer->WriteText(Text));

	return DcOk();
}

FDcResult HandlerTransientObjectDeserialize(FDcDeserializeContext& Ctx)
{
	FDcClassAccess Access{FDcClassAccess::EControl::ReferenceOrNil};
	DC_TRY(Ctx.Writer->WriteClassRootAccess(Access));

	void* Ptr;
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerOrNil(Ctx.Reader, Ptr));
	if (Ptr == nullptr)
	{
		DC_TRY(Ctx.Writer->WriteNil());
	}
	else
	{
		DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Ptr));
		DC_TRY(Ctx.Writer->WriteObjectReference((UObject*)Ptr));
	}

	DC_TRY(Ctx.Writer->WriteClassEndAccess(Access));
	return DcOk();
}

FDcResult HandlerTransientClassDeserialize(FDcDeserializeContext& Ctx)
{
	void* Ptr;
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerOrNil(Ctx.Reader, Ptr));
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Ptr));
	DC_TRY(Ctx.Writer->WriteClassReference((UClass*)Ptr));

	return DcOk();
}

FDcResult HandlerTransientSoftObjectDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteSoftObjectReference(FSoftObjectPtr(FSoftObjectPath{Value})));
	}
	else
	{
		void* Ptr;
		DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, Ptr));
		DC_TRY(Ctx.Writer->WriteSoftObjectReference(FSoftObjectPtr((UObject*)Ptr)));
	}

	return DcOk();
}

FDcResult HandlerTransientSoftClassDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteSoftClassReference(FSoftObjectPtr(FSoftObjectPath{Value})));
	}
	else
	{
		void* Ptr;
		DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, Ptr));
		DC_TRY(Ctx.Writer->WriteSoftClassReference(FSoftObjectPtr((UObject*)Ptr)));
	}

	return DcOk();
}

FDcResult HandlerTransientWeakObjectDeserialize(FDcDeserializeContext& Ctx)
{
	using DcSerDeCommon::FWeakObjectPtrAccess;

	FWeakObjectPtr Value;
	FWeakObjectPtrAccess& ValueAccess = (FWeakObjectPtrAccess&)Value;
	DC_TRY(DcMsgPackHandlersDetails::ReadTransientWeakObjectPtr(Ctx.Reader, ValueAccess));

	DC_TRY(Ctx.Writer->WriteWeakObjectReference(Value));
	return DcOk();
}

FDcResult HandlerTransientLazyObjectDeserialize(FDcDeserializeContext& Ctx)
{
	FDcMsgPackReader* Reader;
	DC_TRY(DcCastReader(Ctx.Reader, Reader));

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::Extension)
	{
		uint8 Type;
		FGuid Guid;
		DC_TRY(Reader->ReadFixExt16(&Type, (FDcBytes16*)(&Guid)));

		FLazyObjectPtr Value;
		Value = Guid;
		DC_TRY(Ctx.Writer->WriteLazyObjectReference(Value));
	}
	else
	{
		void* Ptr;
		DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, Ptr));

		DC_TRY(Ctx.Writer->WriteLazyObjectReference(FLazyObjectPtr((UObject*)Ptr)));
	}

	
	return DcOk();
}

FDcResult HandlerTransientInterfaceDeserialize(FDcDeserializeContext& Ctx)
{
	void* ObjectPtr;
	void* InterfacePtr;

	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, ObjectPtr));
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, InterfacePtr));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	FScriptInterface Value((UObject*)ObjectPtr, InterfacePtr);
	DC_TRY(Ctx.Writer->WriteInterfaceReference(Value));
	
	return DcOk();
}

FDcResult HandlerTransientDelegateDeserialize(FDcDeserializeContext& Ctx)
{
	using DcSerDeCommon::FScriptDelegateAccess;

	FScriptDelegate Value;
	FScriptDelegateAccess& ValueAccess = (FScriptDelegateAccess&)(Value);

	DC_TRY(DcMsgPackHandlersDetails::ReadTransientScriptDelegate(Ctx.Reader, ValueAccess));
	DC_TRY(Ctx.Writer->WriteDelegate(Value));

	return DcOk();
}

FDcResult HandlerTransientMulticastInlineDelegateDeserialize(FDcDeserializeContext& Ctx)
{
	using DcSerDeCommon::FMulticastScriptDelegateAccess;

	FMulticastScriptDelegate Value;
	FMulticastScriptDelegateAccess& ValueAccess = (FMulticastScriptDelegateAccess&)Value;

	DC_TRY(DcMsgPackHandlersDetails::ReadTransientMulticastScriptDelegate(Ctx.Reader, ValueAccess));
	DC_TRY(Ctx.Writer->WriteMulticastInlineDelegate(Value));

	return DcOk();
}

FDcResult HandlerTransientMulticastSparseDelegateDeserialize(FDcDeserializeContext& Ctx)
{
	using DcSerDeCommon::FMulticastScriptDelegateAccess;

	FMulticastScriptDelegate Value;
	FMulticastScriptDelegateAccess& ValueAccess = (FMulticastScriptDelegateAccess&)Value;

	DC_TRY(DcMsgPackHandlersDetails::ReadTransientMulticastScriptDelegate(Ctx.Reader, ValueAccess));
	DC_TRY(Ctx.Writer->WriteMulticastSparseDelegate(Value));

	return DcOk();
}

FDcResult HandlerTransientFieldPathDeserialize(FDcDeserializeContext& Ctx)
{
	void* FieldPtr;
	DC_TRY(DcMsgPackHandlersDetails::ReadPointerRaw(Ctx.Reader, FieldPtr));

	FFieldPath Value((FField*)FieldPtr);
	DC_TRY(Ctx.Writer->WriteFieldPath(Value));

	return DcOk();
}

FDcResult HandlerTransientEnumDeserialize(FDcDeserializeContext& Ctx)
{
	uint64 Value;
	DC_TRY(Ctx.Reader->ReadUInt64(&Value));

	FDcEnumData EnumValue;
	EnumValue.Unsigned64 = Value;
	DC_TRY(Ctx.Writer->WriteEnum(EnumValue));

	return DcOk();
}
} // namespace DcMsgPackHandlers
