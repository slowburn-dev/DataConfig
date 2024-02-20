#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"

template<typename TCtx>
FORCEINLINE_DEBUGGABLE FDcResult DcHandlerPipeScalar(TCtx& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	DC_TRY(DcSerDeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer));
	return DcOk();
}

template<typename TCtx,
	typename TReader,
	typename TWriter,
	EDcDataEntry EntryEnd,
	FDcResult (*Recurse)(TCtx&),
	FDcResult (TReader::*ReadMethodStart)(),
	FDcResult (TReader::*ReadMethodEnd)(),
	FDcResult (TWriter::*WriteMethodStart)(),
	FDcResult (TWriter::*WriteMethodEnd)()>
FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerPipeLinearContainer(
	TCtx& Ctx
)
{
	DC_TRY((Ctx.Reader->*ReadMethodStart)());
	DC_TRY((Ctx.Writer->*WriteMethodStart)());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EntryEnd)
			break;

		DC_TRY(Recurse(Ctx));
	}

	DC_TRY((Ctx.Reader->*ReadMethodEnd)());
	DC_TRY((Ctx.Writer->*WriteMethodEnd)());

	return DcOk();
}

template<typename TCtx,
	typename TReader,
	typename TWriter,
	FDcResult (*Recurse)(TCtx&)>
FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerPipeMap(
	TCtx& Ctx
)
{
	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
			break;

		DC_TRY(Recurse(Ctx));
		DC_TRY(Recurse(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

template<typename TCtx,
	typename TReader,
	typename TWriter,
	FDcResult (*Recurse)(TCtx&)>
FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerPipeStringKeyMap(
	TCtx& Ctx
)
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

		DC_TRY(Recurse(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

template<typename TCtx,
	typename TReader,
	typename TWriter,
	FDcResult (*Recurse)(TCtx&)>
FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerPipeStruct(
	TCtx& Ctx
)
{
	FDcStructAccess Access;
	DC_TRY(Ctx.Reader->ReadStructRootAccess(Access));
	DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::StructEnd)
			break;

		FName FieldName;
		DC_TRY(Ctx.Reader->ReadName(&FieldName));
		DC_TRY(Ctx.Writer->WriteName(FieldName));

		DC_TRY(Recurse(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadStructEndAccess(Access));
	DC_TRY(Ctx.Writer->WriteStructEndAccess(Access));

	return DcOk();
}

template<typename TCtx,
	typename TReader,
	typename TWriter,
	FDcResult (*Recurse)(TCtx&)>
FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerPipeClass(
	TCtx& Ctx
)
{
	FDcClassAccess Access;
	DC_TRY(Ctx.Reader->ReadClassRootAccess(Access));
	DC_TRY(Ctx.Writer->WriteClassRootAccess(Access));

	if (Access.Control == FDcClassAccess::EControl::ReferenceOrNone)
	{
		EDcDataEntry Next;
		DC_TRY(Ctx.Reader->PeekRead(&Next));
		DC_TRY(DcSerDeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer));
	}
	else
	{
		EDcDataEntry CurPeek;
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
			if (CurPeek == EDcDataEntry::ClassEnd)
				break;

			FName FieldName;
			DC_TRY(Ctx.Reader->ReadName(&FieldName));
			DC_TRY(Ctx.Writer->WriteName(FieldName));

			DC_TRY(Recurse(Ctx));
		}
	}

	DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));
	DC_TRY(Ctx.Writer->WriteClassEndAccess(Access));

	return DcOk();
}

template<typename TCtx,
	typename TReader,
	typename TWriter,
	FDcResult (*Recurse)(TCtx&)>
FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerPipeOptional(
	TCtx& Ctx
)
{
	DC_TRY(Ctx.Reader->ReadOptionalRoot());
	DC_TRY(Ctx.Writer->WriteOptionalRoot());

	EDcDataEntry CurPeek;
	DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
	if (CurPeek == EDcDataEntry::None)
	{
		DC_TRY(Ctx.Reader->ReadNone());
		DC_TRY(Ctx.Writer->WriteNone());
	}
	else
	{
		DC_TRY(Recurse(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadOptionalEnd());
	DC_TRY(Ctx.Writer->WriteOptionalEnd());

	return DcOk();
}

