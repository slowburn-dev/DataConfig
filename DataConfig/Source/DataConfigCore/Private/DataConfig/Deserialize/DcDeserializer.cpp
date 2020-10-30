#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"

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


FDcResult FDcDeserializer::Deserialize(FDcDeserializeContext& Ctx)
{
	check(Ctx.Deserializer == this);
	check(Ctx.Reader != nullptr);
	check(Ctx.Writer != nullptr);
	check(Ctx.Properties.Num() > 0);

	for (auto& PredPair : PredicatedDeserializers)
	{
		if (PredPair.Key.Execute(Ctx) == EDcDeserializePredicateResult::Process)
		{
			return _ExecuteDeserializeHandler(Ctx, PredPair.Value);
		}
	}

	UField* Property = Ctx.TopProperty();
	FDcDeserializeDelegate* HandlerPtr = DirectDeserializersMap.Find(Property->GetClass());
	if (HandlerPtr == nullptr)
	{
		return DC_FAIL(DcDDeserialize, NoMatchingHandler)
			<< Ctx.TopProperty()->GetFName() << Ctx.TopProperty()->GetClass()->GetFName();
	}

	return _ExecuteDeserializeHandler(Ctx, *HandlerPtr);
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

