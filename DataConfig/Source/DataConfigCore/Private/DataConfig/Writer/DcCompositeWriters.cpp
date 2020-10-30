#include "DataConfig/Writer/DcCompositeWriters.h"

template<typename TMethod, typename... TArgs>
FORCEINLINE FDcResult CompositeDispatch(FDcWeakCompositeWriter* Self, TMethod Method, TArgs&&... Args)
{
	for (FDcWriter* Writer : Self->Writers)
		DC_TRY((Writer->*Method)(Forward<TArgs>(Args)...));

	return DcOk();
}

FDcResult FDcWeakCompositeWriter::WriteNext(EDcDataEntry Next)
{
	return CompositeDispatch(this, &FDcWriter::WriteNext, Next);
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

FDcResult FDcWeakCompositeWriter::WriteStructRoot(const FName& Name)
{
	return CompositeDispatch(this, &FDcWriter::WriteStructRoot, Name);
}

FDcResult FDcWeakCompositeWriter::WriteStructEnd(const FName& Name)
{
	return CompositeDispatch(this, &FDcWriter::WriteStructEnd, Name);
}

FDcResult FDcWeakCompositeWriter::WriteClassRoot(const FDcClassPropertyStat& Class)
{
	return CompositeDispatch(this, &FDcWriter::WriteClassRoot, Class);
}

FDcResult FDcWeakCompositeWriter::WriteClassEnd(const FDcClassPropertyStat& Class)
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

FDcResult FDcWeakCompositeWriter::WriteNil()
{
	return CompositeDispatch(this, &FDcWriter::WriteNil);
}

FDcResult FDcWeakCompositeWriter::WriteReference(UObject* Value)
{
	return CompositeDispatch(this, &FDcWriter::WriteReference, Value);
}

