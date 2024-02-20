#include "DataConfig/Serialize/Handlers/MsgPack/DcMsgPackTransientSerializers.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/SerDe/DcSerDeCommon.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"

namespace DcMsgPackHandlersDetails
{

static FORCEINLINE_DEBUGGABLE FDcResult WritePointer(FDcWriter* Writer, void* Value)
{
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value));
#if PLATFORM_64BITS
	return Writer->WriteUInt64((uint64)Value);
#else
	return Writer->WriteUInt32((uint32)Value);
#endif
}

static FORCEINLINE_DEBUGGABLE FDcResult WriteTransientName(FDcWriter* Writer, const FName& Name)
{
	DC_TRY(Writer->WriteArrayRoot());
	DC_TRY(Writer->WriteUInt32(Name.GetComparisonIndex().ToUnstableInt()));
	DC_TRY(Writer->WriteUInt32(Name.GetDisplayIndex().ToUnstableInt()));
	DC_TRY(Writer->WriteInt32(Name.GetNumber()));
	DC_TRY(Writer->WriteArrayEnd());
	return DcOk();
}

template<typename TDelegate>
static FORCEINLINE_DEBUGGABLE FDcResult WriteTransientScriptDelegate(FDcWriter* Writer, DcSerDeCommon::TDelegateAccess<TDelegate>& ValueAccess)
{
	using DcSerDeCommon::FWeakObjectPtrAccess;
	FWeakObjectPtrAccess& WeakAccess = (FWeakObjectPtrAccess&)(ValueAccess.GetObject());

	DC_TRY(Writer->WriteArrayRoot());
	DC_TRY(Writer->WriteInt32(WeakAccess.ObjectIndex));
	DC_TRY(Writer->WriteInt32(WeakAccess.ObjectSerialNumber));
	DC_TRY(WriteTransientName(Writer, ValueAccess.GetFunctionName()));
	DC_TRY(Writer->WriteArrayEnd());

	return DcOk();
}

static FORCEINLINE_DEBUGGABLE FDcResult WriteTransientMulticastScriptDelegate(FDcWriter* Writer, DcSerDeCommon::FMulticastScriptDelegateAccess& ValueAccess)
{
	using DcSerDeCommon::FScriptDelegateAccess;
	using FInvocationAccess = DcSerDeCommon::FMulticastScriptDelegateAccess::TypeInvocationAccess;

	DC_TRY(Writer->WriteArrayRoot());
	for (int Ix = 0; Ix < ValueAccess.GetInvocationList().Num(); Ix++)
	{
		FInvocationAccess& DelegateAccess = (FInvocationAccess&)(ValueAccess.GetInvocationList()[Ix]);
		DC_TRY(WriteTransientScriptDelegate(Writer, DelegateAccess));
	}
	DC_TRY(Writer->WriteArrayEnd());

	return DcOk();
}

	
} // namespace DcMsgPackHandlersDetails

namespace DcMsgPackHandlers
{

FDcResult HandlerTransientNameSerialize(FDcSerializeContext& Ctx)
{
	FName Value;
	DC_TRY(Ctx.Reader->ReadName(&Value));
	DC_TRY(DcMsgPackHandlersDetails::WriteTransientName(Ctx.Writer, Value));

	return DcOk();
}

FDcResult HandlerTransientTextSerialize(FDcSerializeContext& Ctx)
{
	using namespace DcSerDeCommon;

	FText Value;
	DC_TRY(Ctx.Reader->ReadText(&Value));
	FTextAccess& ValueAccess = (FTextAccess&)(Value);

	FSharedRefAccess& SharedRefAccess = (FSharedRefAccess&)ValueAccess.TextData;
	DC_TRY(Ctx.Writer->WriteArrayRoot());
	DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, SharedRefAccess.Object));
	DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, SharedRefAccess.SharedReferenceCount));
	DC_TRY(Ctx.Writer->WriteUInt32(ValueAccess.Flags));
	DC_TRY(Ctx.Writer->WriteArrayEnd());

	return DcOk();
}

FDcResult HandlerTransientObjectSerialize(FDcSerializeContext& Ctx)
{
	FDcClassAccess Access{FDcClassAccess::EControl::ReferenceOrNone};
	DC_TRY(Ctx.Reader->ReadClassRootAccess(Access));

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::None)
	{
		DC_TRY(DcPipe_None(Ctx.Reader, Ctx.Writer));
	}
	else if (Next == EDcDataEntry::ObjectReference)
	{
		UObject* Value;
		DC_TRY(Ctx.Reader->ReadObjectReference(&Value));
		DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, Value));
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::None << EDcDataEntry::ObjectReference << Next;
	}

	DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));

	return DcOk();
}

FDcResult HandlerTransientClassSerialize(FDcSerializeContext& Ctx)
{
	UClass* Value;
	DC_TRY(Ctx.Reader->ReadClassReference(&Value));
	DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, Value));

	return DcOk();
}

FDcResult HandlerTransientSoftObjectSerialize(FDcSerializeContext& Ctx)
{
	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftObjectReference(&Value));

	if (Value.IsStale())
	{
		DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	}
	else
	{
		DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, Value.Get()));
	}

	return DcOk();
}

FDcResult HandlerTransientSoftClassSerialize(FDcSerializeContext& Ctx)
{
	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftClassReference(&Value));

	if (Value.IsStale())
	{
		DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	}
	else
	{
		DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, Value.Get()));
	}

	return DcOk();
}

FDcResult HandlerTransientWeakObjectSerialize(FDcSerializeContext& Ctx)
{
	using DcSerDeCommon::FWeakObjectPtrAccess;
	FWeakObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadWeakObjectReference(&Value));

	FWeakObjectPtrAccess& ValueAccess = (FWeakObjectPtrAccess&)Value;
	DC_TRY(Ctx.Writer->WriteArrayRoot());
	DC_TRY(Ctx.Writer->WriteInt32(ValueAccess.ObjectIndex));
	DC_TRY(Ctx.Writer->WriteInt32(ValueAccess.ObjectSerialNumber));
	DC_TRY(Ctx.Writer->WriteArrayEnd());

	return DcOk();
}

FDcResult HandlerTransientLazyObjectSerialize(FDcSerializeContext& Ctx)
{
	FDcMsgPackWriter* Writer;
	DC_TRY(DcCastWriter(Ctx.Writer, Writer));

	FLazyObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadLazyObjectReference(&Value));

	if (Value.IsStale())
	{
		const FGuid& Guid = Value.GetUniqueID().GetGuid();
		static_assert(sizeof(FGuid) == sizeof(FDcBytes16), "expect same layout FGuid == FDcBytes16");
		DC_TRY(Writer->WriteFixExt16(0x11, (FDcBytes16&)(Guid)));
		return DcOk();
	}
	else
	{
		DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, Value.Get()));
	}

	return DcOk();
}


FDcResult HandlerTransientInterfaceSerialize(FDcSerializeContext& Ctx)
{
	using DcSerDeCommon::FScriptInterfaceAccess;

	FScriptInterface Value;
	DC_TRY(Ctx.Reader->ReadInterfaceReference(&Value));

	FScriptInterfaceAccess& ValueAccess = (FScriptInterfaceAccess&)Value;
	DC_TRY(Ctx.Writer->WriteArrayRoot());
	DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, ValueAccess.ObjectPointer));
	DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, ValueAccess.InterfacePointer));
	DC_TRY(Ctx.Writer->WriteArrayEnd());

	return DcOk();
}

FDcResult HandlerTransientDelegateSerialize(FDcSerializeContext& Ctx)
{
	using DcSerDeCommon::FScriptDelegateAccess;

	FScriptDelegate Value;
	DC_TRY(Ctx.Reader->ReadDelegate(&Value));

	FScriptDelegateAccess& ValueAccess = (FScriptDelegateAccess&)(Value);
	DC_TRY(DcMsgPackHandlersDetails::WriteTransientScriptDelegate(Ctx.Writer, ValueAccess));

	return DcOk();
}

FDcResult HandlerTransientMulticastInlineDelegateSerialize(FDcSerializeContext& Ctx)
{
	using DcSerDeCommon::FMulticastScriptDelegateAccess;

	FMulticastScriptDelegate Value;
	DC_TRY(Ctx.Reader->ReadMulticastInlineDelegate(&Value));

	FMulticastScriptDelegateAccess& ValueAccess = (FMulticastScriptDelegateAccess&)Value;
	DC_TRY(DcMsgPackHandlersDetails::WriteTransientMulticastScriptDelegate(Ctx.Writer, ValueAccess));

	return DcOk();
}

FDcResult HandlerTransientMulticastSparseDelegateSerialize(FDcSerializeContext& Ctx)
{
	using DcSerDeCommon::FMulticastScriptDelegateAccess;

	FMulticastScriptDelegate Value;
	DC_TRY(Ctx.Reader->ReadMulticastSparseDelegate(&Value));

	FMulticastScriptDelegateAccess& ValueAccess = (FMulticastScriptDelegateAccess&)Value;
	DC_TRY(DcMsgPackHandlersDetails::WriteTransientMulticastScriptDelegate(Ctx.Writer, ValueAccess));

	return DcOk();
}

FDcResult HandlerTransientFieldPathSerialize(FDcSerializeContext& Ctx)
{
	FFieldPath Value;
	DC_TRY(Ctx.Reader->ReadFieldPath(&Value));

	static_assert(sizeof(FFieldPath) == sizeof(TFieldPath<FProperty>), "TFieldPath<FProperty> stale");
	TFieldPath<FProperty>& ValueAccess = (TFieldPath<FProperty>&)Value;

	DC_TRY(DcMsgPackHandlersDetails::WritePointer(Ctx.Writer, ValueAccess.Get()));
	return DcOk();
}

FDcResult HandlerTransientEnumSerialize(FDcSerializeContext& Ctx)
{
	FDcEnumData Value;
	DC_TRY(Ctx.Reader->ReadEnum(&Value));
	DC_TRY(Ctx.Writer->WriteUInt64(Value.Unsigned64));
	return DcOk();
}

} // namespace DcMsgPackHandlers

