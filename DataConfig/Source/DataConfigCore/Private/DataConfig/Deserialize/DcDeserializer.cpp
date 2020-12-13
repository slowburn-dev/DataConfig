#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "Misc/ScopeExit.h"

namespace DcDeserializerDetails
{

static FDcResult ExecuteDeserializeHandler(FDcDeserializeContext& Ctx, FDcDeserializeDelegate& Handler)
{
	EDcDeserializeResult HandlerRet;
	DC_TRY(Handler.Execute(Ctx, HandlerRet));
	check(HandlerRet != EDcDeserializeResult::Unknown);

	if (HandlerRet == EDcDeserializeResult::CanNotProcess)
	{
		return DC_FAIL(DcDDeserialize, NoMatchingHandler)
			<< Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}
	else
	{
		return DcOk();
	}
}

static FDcResult DeserializeBody(FDcDeserializer& Self, FDcDeserializeContext& Ctx)
{
	//	use predicated deserializers first, if it's not handled then try direct handlers
	for (auto& PredPair : Self.PredicatedDeserializers)
	{
		if (PredPair.Key.Execute(Ctx) == EDcDeserializePredicateResult::Process)
		{
			return ExecuteDeserializeHandler(Ctx, PredPair.Value);
		}
	}

	FFieldVariant& Property = Ctx.TopProperty();
	FDcDeserializeDelegate* HandlerPtr;
	if (Property.IsUObject())
	{
		UClass* Class = CastChecked<UClass>(Property.ToUObjectUnsafe());
		HandlerPtr = Self.UClassDeserializerMap.Find(Class);
		if (HandlerPtr == nullptr)
			return DC_FAIL(DcDDeserialize, NoMatchingHandler)
				<< Ctx.TopProperty().GetFName() << Class->GetFName();
	}
	else
	{
		FField* Field = Property.ToFieldUnsafe();
		check(Field->IsValidLowLevel());
		FFieldClass* FieldClass = Field->GetClass();
		HandlerPtr = Self.FieldClassDeserializerMap.Find(FieldClass);
		if (HandlerPtr == nullptr)
			return DC_FAIL(DcDDeserialize, NoMatchingHandler)
				<< Ctx.TopProperty().GetFName() << FieldClass->GetFName();
	}

	return ExecuteDeserializeHandler(Ctx, *HandlerPtr);
}

}	// namespace DcDeserializerDetails

FDcResult FDcDeserializer::Deserialize(FDcDeserializeContext& Ctx)
{
	using ECtxState = FDcDeserializeContext::EState;
	if (Ctx.State == ECtxState::Uninitialized)
	{
		return DC_FAIL(DcDDeserialize, NotPrepared);
	}
	else if (Ctx.State == ECtxState::Ready)
	{
		Ctx.State = FDcDeserializeContext::EState::DeserializeInProgress;

		ON_SCOPE_EXIT
		{
			Ctx.State = ECtxState::DeserializeEnded;
		};

		return DcDeserializerDetails::DeserializeBody(*this, Ctx);
	}
	else if (Ctx.State == FDcDeserializeContext::EState::DeserializeInProgress)
	{
		return DcDeserializerDetails::DeserializeBody(*this, Ctx);
	}
	else
	{
		return DcNoEntry();
	}
}

void FDcDeserializer::AddDirectHandler(UClass* PropertyClass, FDcDeserializeDelegate&& Delegate)
{
	check(!UClassDeserializerMap.Contains(PropertyClass));
	UClassDeserializerMap.Add(PropertyClass, MoveTemp(Delegate));
}

void FDcDeserializer::AddDirectHandler(FFieldClass* PropertyClass, FDcDeserializeDelegate&& Delegate)
{
	check(!FieldClassDeserializerMap.Contains(PropertyClass));
	FieldClassDeserializerMap.Add(PropertyClass, MoveTemp(Delegate));
}

void FDcDeserializer::AddPredicatedHandler(FDcDeserializePredicate&& Predicate, FDcDeserializeDelegate&& Delegate)
{
	PredicatedDeserializers.Add(MakeTuple(MoveTemp(Predicate), MoveTemp(Delegate)));
}

