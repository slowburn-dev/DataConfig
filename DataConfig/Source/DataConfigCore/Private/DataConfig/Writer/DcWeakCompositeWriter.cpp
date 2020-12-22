#include "DataConfig/Writer/DcWeakCompositeWriter.h"

template<typename TMethod, typename... TArgs>
FORCEINLINE FDcResult CompositeDispatch(FDcWeakCompositeWriter* Self, TMethod Method, TArgs&&... Args)
{
	FDcEnv& Env = DcEnv();

	for (FDcWriter* Writer : Self->Writers)
	{
		DC_TRY((Writer->*Method)(Forward<TArgs>(Args)...));
	}

	return DcOk();
}

FDcResult FDcWeakCompositeWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	return CompositeDispatch(this, &FDcWriter::PeekWrite, Next, bOutOk);
}

FDcResult FDcWeakCompositeWriter::WriteBool(bool Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteBool, Value);
}

FDcResult FDcWeakCompositeWriter::WriteName(const FName& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteName, Value);
}

FDcResult FDcWeakCompositeWriter::WriteString(const FString& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteString, Value);
}

FDcResult FDcWeakCompositeWriter::WriteText(const FText& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteText, Value);
}

FDcResult FDcWeakCompositeWriter::WriteEnum(const FDcEnumData& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteEnum, Value);
}

FDcResult FDcWeakCompositeWriter::WriteStructRoot(const FDcStructStat& Struct)
{
	return CompositeDispatch(this, &FDcWriter::WriteStructRoot, Struct);
}

FDcResult FDcWeakCompositeWriter::WriteStructEnd(const FDcStructStat& Struct)
{
	return CompositeDispatch(this, &FDcWriter::WriteStructEnd, Struct);
}

FDcResult FDcWeakCompositeWriter::WriteClassRoot(const FDcClassStat& Class)
{
	return CompositeDispatch(this, &FDcWriter::WriteClassRoot, Class);
}

FDcResult FDcWeakCompositeWriter::WriteClassEnd(const FDcClassStat& Class)
{
	return CompositeDispatch(this, &FDcWriter::WriteClassEnd, Class);
}

FDcResult FDcWeakCompositeWriter::WriteMapRoot()
{
	return CompositeDispatch(this, &FDcWriter::WriteMapRoot);
}

FDcResult FDcWeakCompositeWriter::WriteMapEnd()
{
	return CompositeDispatch(this, &FDcWriter::WriteMapEnd);
}

FDcResult FDcWeakCompositeWriter::WriteArrayRoot()
{
	return CompositeDispatch(this, &FDcWriter::WriteArrayRoot);
}

FDcResult FDcWeakCompositeWriter::WriteArrayEnd()
{
	return CompositeDispatch(this, &FDcWriter::WriteArrayEnd);
}

FDcResult FDcWeakCompositeWriter::WriteSetRoot()
{
	return CompositeDispatch(this, &FDcWriter::WriteSetRoot);
}

FDcResult FDcWeakCompositeWriter::WriteSetEnd()
{
	return CompositeDispatch(this, &FDcWriter::WriteSetEnd);
}

FDcResult FDcWeakCompositeWriter::WriteNil()
{
	return CompositeDispatch(this, &FDcWriter::WriteNil);
}

FDcResult FDcWeakCompositeWriter::WriteObjectReference(const UObject* Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteObjectReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteClassReference(const UClass* Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteClassReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteInt8(const int8& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteInt8, Value);
}

FDcResult FDcWeakCompositeWriter::WriteInt16(const int16& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteInt16, Value);
}

FDcResult FDcWeakCompositeWriter::WriteInt32(const int32& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteInt32, Value);
}

FDcResult FDcWeakCompositeWriter::WriteInt64(const int64& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteInt64, Value);
}

FDcResult FDcWeakCompositeWriter::WriteUInt8(const uint8& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteUInt8, Value);
}

FDcResult FDcWeakCompositeWriter::WriteUInt16(const uint16& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteUInt16, Value);
}

FDcResult FDcWeakCompositeWriter::WriteUInt32(const uint32& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteUInt32, Value);
}

FDcResult FDcWeakCompositeWriter::WriteUInt64(const uint64& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteUInt64, Value);
}

FDcResult FDcWeakCompositeWriter::WriteFloat(const float& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteFloat, Value);
}

FDcResult FDcWeakCompositeWriter::WriteDouble(const double& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteDouble, Value);
}

FDcResult FDcWeakCompositeWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteWeakObjectReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteLazyObjectReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteSoftObjectReference(const FSoftObjectPath& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteSoftObjectReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteSoftClassReference(const FSoftClassPath& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteSoftClassReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteInterfaceReference(const FScriptInterface& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteInterfaceReference, Value);
}

FDcResult FDcWeakCompositeWriter::WriteFieldPath(const FFieldPath& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteFieldPath, Value);
}

FDcResult FDcWeakCompositeWriter::WriteDelegate(const FScriptDelegate& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteDelegate, Value);
}

FDcResult FDcWeakCompositeWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteMulticastInlineDelegate, Value);
}

FDcResult FDcWeakCompositeWriter::WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteMulticastSparseDelegate, Value);
}

FDcResult FDcWeakCompositeWriter::WriteBlob(const FDcBlobViewData& Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteBlob, Value);
}

void FDcWeakCompositeWriter::FormatDiagnostic(FDcDiagnostic& Diag)
{
	for (FDcWriter* Writer : Writers)
		Writer->FormatDiagnostic(Diag);
}
