#include "DataConfig/Deserialize/Handlers/Json/DcJsonContainerDeserializers.h"
#include "UObject/UnrealType.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcJsonHandlers {

template<EDcDataEntry EntryStart,
	FDcResult (FDcPropertyWriter::*MethodStart)(),
	FDcResult (FDcPropertyWriter::*MethodEnd)()>
FORCEINLINE static FDcResult _HandlerLinearContainerDeserialize(
	FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY((Ctx.Writer->*MethodStart)());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadArrayEnd());
	DC_TRY((Ctx.Writer->*MethodEnd)());

	return DcOk();
}

FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx)
{
	return _HandlerLinearContainerDeserialize<EDcDataEntry::ArrayRoot, &FDcPropertyWriter::WriteArrayRoot, &FDcPropertyWriter::WriteArrayEnd>(Ctx);
}

FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx)
{
	return _HandlerLinearContainerDeserialize<EDcDataEntry::SetRoot, &FDcPropertyWriter::WriteSetRoot, &FDcPropertyWriter::WriteSetEnd>(Ctx);
}

FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
			break;

		//	allow only string key
		FString Key;
		DC_TRY(Ctx.Reader->ReadString(&Key));
		DC_TRY(Ctx.Writer->WriteString(Key));

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

}	// namespace DcJsonHandlers

