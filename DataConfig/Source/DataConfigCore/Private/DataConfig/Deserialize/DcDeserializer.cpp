#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "Misc/ScopeExit.h"

static FDcResult _ExecuteDeserializeHandler(FDcDeserializeContext& Ctx, FDcDeserializeDelegate& Handler)
{
	EDcDeserializeResult HandlerRet;
	DC_TRY(Handler.Execute(Ctx, HandlerRet));
	check(HandlerRet != EDcDeserializeResult::Unknown);

	if (HandlerRet == EDcDeserializeResult::CanNotProcess)
	{
		return DC_FAIL(DcDDeserialize, NoMatchingHandler)
			<< Ctx.TopProperty()->GetFName() << Ctx.TopProperty()->GetClass()->GetFName();
	}
	else
	{
		return DcOk();
	}
}

static FDcResult _DeserializeBody(FDcDeserializer& Self, FDcDeserializeContext& Ctx)
{
	for (auto& PredPair : Self.PredicatedDeserializers)
	{
		if (PredPair.Key.Execute(Ctx) == EDcDeserializePredicateResult::Process)
		{
			return _ExecuteDeserializeHandler(Ctx, PredPair.Value);
		}
	}

	UField* Property = Ctx.TopProperty();
	FDcDeserializeDelegate* HandlerPtr = Self.DirectDeserializersMap.Find(Property->GetClass());
	if (HandlerPtr == nullptr)
	{
		return DC_FAIL(DcDDeserialize, NoMatchingHandler)
			<< Ctx.TopProperty()->GetFName() << Ctx.TopProperty()->GetClass()->GetFName();
	}

	return _ExecuteDeserializeHandler(Ctx, *HandlerPtr);
}

FDcResult FDcDeserializer::Deserialize(FDcDeserializeContext& Ctx)
{
	using ECtxState = FDcDeserializeContext::EState;
	if (Ctx.State == ECtxState::Uninitialized)
	{
		checkf(false, TEXT("need `Ctx.Prepare()`"));
		return DC_FAIL(DcDCommon, Unreachable);
	}
	else if (Ctx.State == ECtxState::Ready)
	{
		Ctx.State = FDcDeserializeContext::EState::DeserializeInProgress;

		ON_SCOPE_EXIT
		{
			Ctx.State = ECtxState::DeserializeEnded;
		};

		return _DeserializeBody(*this, Ctx);
	}
	else if (Ctx.State == FDcDeserializeContext::EState::DeserializeInProgress)
	{
		return _DeserializeBody(*this, Ctx);
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

void FDcDeserializer::AddDirectHandler(UClass* PropertyClass, FDcDeserializeDelegate&& Delegate)
{
	check(!DirectDeserializersMap.Contains(PropertyClass));
	DirectDeserializersMap.Add(PropertyClass, MoveTemp(Delegate));
}

void FDcDeserializer::AddPredicatedHandler(FDcDeserializePredicate&& Predicate, FDcDeserializeDelegate&& Delegate)
{
	PredicatedDeserializers.Add(MakeTuple(MoveTemp(Predicate), MoveTemp(Delegate)));
}

