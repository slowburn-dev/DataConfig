#include "Writer/DcCompositeWriters.h"

namespace DataConfig
{

template<typename TMethod, typename... TArgs>
FORCEINLINE FResult CompositeDispatch(FWeakCompositeWriter* Self, TMethod Method, TArgs&&... Args)
{
	for (FWriter* Writer : Self->Writers)
		TRY((Writer->*Method)(Forward<TArgs>(Args)...));

	return Ok();
}

FResult FWeakCompositeWriter::Peek(EDataEntry Next)
{
	return CompositeDispatch(this, &FWriter::Peek, Next);
}

FResult FWeakCompositeWriter::WriteBool(bool Value)
{
	return CompositeDispatch(this, &FWriter::WriteBool, Value);
}

FResult FWeakCompositeWriter::WriteName(const FName& Value)
{
	return CompositeDispatch(this, &FWriter::WriteName, Value);
}

FResult FWeakCompositeWriter::WriteString(const FString& Value)
{
	return CompositeDispatch(this, &FWriter::WriteString, Value);
}

FResult FWeakCompositeWriter::WriteStructRoot(const FName& Name)
{
	return CompositeDispatch(this, &FWriter::WriteStructRoot, Name);
}

FResult FWeakCompositeWriter::WriteStructEnd(const FName& Name)
{
	return CompositeDispatch(this, &FWriter::WriteStructEnd, Name);
}

DataConfig::FResult FWeakCompositeWriter::WriteClassRoot(const FClassPropertyStat& Class)
{
	return CompositeDispatch(this, &FWriter::WriteClassRoot, Class);
}

DataConfig::FResult FWeakCompositeWriter::WriteClassEnd(const FClassPropertyStat& Class)
{
	return CompositeDispatch(this, &FWriter::WriteClassEnd, Class);
}

FResult FWeakCompositeWriter::WriteMapRoot()
{
	return CompositeDispatch(this, &FWriter::WriteMapRoot);
}

FResult FWeakCompositeWriter::WriteMapEnd()
{
	return CompositeDispatch(this, &FWriter::WriteMapEnd);
}

FResult FWeakCompositeWriter::WriteArrayRoot()
{
	return CompositeDispatch(this, &FWriter::WriteArrayRoot);
}

FResult FWeakCompositeWriter::WriteArrayEnd()
{
	return CompositeDispatch(this, &FWriter::WriteArrayEnd);
}


}	// namespace DataConfig
