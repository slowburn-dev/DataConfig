#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"

namespace DataConfig
{

static FDcResult ExecuteDeserializeHandler(FDeserializeContext& Ctx, FDeserializeDelegate& Handler)
{
	EDeserializeResult HandlerRet;
	DC_TRY(Handler.Execute(Ctx, HandlerRet));
	check(HandlerRet != EDeserializeResult::Unknown);

	if (HandlerRet == EDeserializeResult::CanNotProcess)
	{
		return DcFail(DC_DIAG(DDeserialize, NoMatchingHandler))
			<< Ctx.TopProperty()->GetFName() << Ctx.TopProperty()->GetClass()->GetFName();
	}
	else
	{
		return DcOk();
	}
}


FDcResult FDeserializer::Deserialize(FDeserializeContext& Ctx)
{
	check(Ctx.Deserializer == this);
	check(Ctx.Reader != nullptr);
	check(Ctx.Writer != nullptr);
	check(Ctx.Properties.Num() > 0);

	for (auto& PredPair : PredicatedDeserializers)
	{
		if (PredPair.Key.Execute(Ctx) == EDeserializePredicateResult::Process)
		{
			return ExecuteDeserializeHandler(Ctx, PredPair.Value);
		}
	}

	UField* Property = Ctx.TopProperty();
	FDeserializeDelegate* HandlerPtr = DirectDeserializersMap.Find(Property->GetClass());
	if (HandlerPtr == nullptr)
	{
		return DcFail(DC_DIAG(DDeserialize, NoMatchingHandler))
			<< Ctx.TopProperty()->GetFName() << Ctx.TopProperty()->GetClass()->GetFName();
	}

	return ExecuteDeserializeHandler(Ctx, *HandlerPtr);
}

void FDeserializer::AddDirectHandler(UClass* PropertyClass, FDeserializeDelegate&& Delegate)
{
	check(!DirectDeserializersMap.Contains(PropertyClass));
	DirectDeserializersMap.Add(PropertyClass, MoveTemp(Delegate));
}

void FDeserializer::AddPredicatedHandler(FDeserializePredicate&& Predicate, FDeserializeDelegate&& Delegate)
{
	PredicatedDeserializers.Add(MakeTuple(MoveTemp(Predicate), MoveTemp(Delegate)));
}

} // namespace DataConfig


