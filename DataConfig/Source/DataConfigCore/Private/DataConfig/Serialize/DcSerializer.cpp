#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "Misc/ScopeExit.h"

namespace DcSerializerDetails
{

static FORCEINLINE FDcResult ExecuteSerializeHandler(FDcSerializeContext& Ctx, FDcSerializeDelegate& Handler)
{
	if (!Handler.IsBound())
		return DC_FAIL(DcDCommon, StaleDelegate);

	return Handler.Execute(Ctx);
}

static FDcResult SerializeBody(FDcSerializer* Self, FDcSerializeContext& Ctx)
{
	//	try predicated serializers first, if not handled then try direct handlers
	for (auto& PredPair : Self->PredicatedSerializers)
	{
		if (!PredPair.Key.IsBound())
			return DC_FAIL(DcDCommon, StaleDelegate);

		if (PredPair.Key.Execute(Ctx) == EDcSerializePredicateResult::Process)
			return ExecuteSerializeHandler(Ctx, PredPair.Value);
	}

	FFieldVariant& Property = Ctx.TopProperty();
	FDcSerializeDelegate* HandlerPtr;
	if (Property.IsUObject())
	{
		UObject* Object = CastChecked<UObject>(Property.ToUObjectUnsafe());
		check(IsValid(Object));
		UClass* Class = Object->GetClass();
		HandlerPtr = Self->UClassSerializerMap.Find(Class);
		if (HandlerPtr == nullptr)
			return DC_FAIL(DcDSerDe, NoMatchingHandler)
				<< Ctx.TopProperty().GetFName() << Class->GetFName();
	}
	else
	{
		FField* Field = Property.ToFieldUnsafe();
		check(Field->IsValidLowLevel());
		FFieldClass* FieldClass = Field->GetClass();
		HandlerPtr = Self->FieldClassSerializerMap.Find(FieldClass);
		if (HandlerPtr == nullptr)
			return DC_FAIL(DcDSerDe, NoMatchingHandler)
				<< Ctx.TopProperty().GetFName() << FieldClass->GetFName();
	}

	return ExecuteSerializeHandler(Ctx, *HandlerPtr);
}

static void AmendDiagnostic(FDcDiagnostic& Diag, FDcSerializeContext& Ctx)
{
	if (Diag.Highlights.IndexOfByPredicate([&Ctx](auto& Highlight){
		return Highlight.Owner == Ctx.Serializer;}) != INDEX_NONE)
		return;

	FDcDiagnosticHighlight Highlight(Ctx.Serializer, TEXT("DcSerializer"));
	TStringBuilder<1024> Sb;
	Sb.Append(TEXT("\n### Properties"));
	{
		int Num = Ctx.Properties.Num();
		if (Num)
		{
			Sb.Appendf(TEXT(" (%d) :"), Num);
			for (FFieldVariant& Field : Ctx.Properties)
			{
				Sb.Append(TEXT("\n  - "));
				Sb.Append(Field.GetName());
			}
		}
		else
		{
			Sb.Append(TEXT(" (0)."));
		}
	}

	Highlight.Formatted = Sb.ToString();
	Diag << MoveTemp(Highlight);
}

} // namespace DcSerializerDetails


FDcResult FDcSerializer::Serialize(FDcSerializeContext& Ctx)
{
	auto _AmendDiag = [](FDcSerializeContext& Ctx)
	{
		FDcDiagnostic& Diag = DcEnv().GetLastDiag();
		DcDiagnosticUtils::AmendDiagnostic(Diag, Ctx.Reader, Ctx.Writer);
		DcSerializerDetails::AmendDiagnostic(Diag, Ctx);
	};

	using ECtxState = FDcSerializeContext::EState;
	if (Ctx.State == ECtxState::Uninitialized)
	{
		return DC_FAIL(DcDSerDe, NotPrepared);
	}
	else if (Ctx.State == ECtxState::Ready)
	{
		Ctx.State = ECtxState::SerializeInProgress;

		ON_SCOPE_EXIT
		{
			Ctx.State = ECtxState::SerializeEnded;
		};

		FDcResult Result = DcSerializerDetails::SerializeBody(this, Ctx);
		if (!Result.Ok()) _AmendDiag(Ctx);

		return Result;
	}
	else if (Ctx.State == ECtxState::SerializeInProgress)
	{
		FDcResult Result = DcSerializerDetails::SerializeBody(this, Ctx);
		if (!Result.Ok()) _AmendDiag(Ctx);

		return Result;
	}
	else
	{
		return DcNoEntry();
	}
}

void FDcSerializer::AddDirectHandler(UClass* PropertyClass, FDcSerializeDelegate&& Delegate)
{
	check(!UClassSerializerMap.Contains(PropertyClass));
	UClassSerializerMap.Add(PropertyClass, MoveTemp(Delegate));
}

void FDcSerializer::AddDirectHandler(FFieldClass* PropertyClass, FDcSerializeDelegate&& Delegate)
{
	check(!FieldClassSerializerMap.Contains(PropertyClass));
	FieldClassSerializerMap.Add(PropertyClass, MoveTemp(Delegate));

}
void FDcSerializer::AddPredicatedHandler(FDcSerializePredicate&& Predicate, FDcSerializeDelegate&& Delegate)
{
	PredicatedSerializers.Add(MakeTuple(MoveTemp(Predicate), MoveTemp(Delegate)));
}

