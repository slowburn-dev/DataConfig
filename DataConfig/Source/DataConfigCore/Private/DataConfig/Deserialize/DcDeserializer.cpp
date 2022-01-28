#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "Misc/ScopeExit.h"
#include "Misc/StringBuilder.h"

namespace DcDeserializerDetails
{

static FORCEINLINE FDcResult ExecuteDeserializeHandler(FDcDeserializeContext& Ctx, FDcDeserializeDelegate& Handler)
{
	if (!Handler.IsBound())
		return DC_FAIL(DcDCommon, StaleDelegate);

	return Handler.Execute(Ctx);
}

static FDcResult DeserializeBody(FDcDeserializer* Self, FDcDeserializeContext& Ctx)
{
	//	use predicated deserializers first, if it's not handled then try direct handlers
	for (auto& PredPair : Self->PredicatedDeserializers)
	{
		if (!PredPair.Key.IsBound())
			return DC_FAIL(DcDCommon, StaleDelegate);

		if (PredPair.Key.Execute(Ctx) == EDcDeserializePredicateResult::Process)
			return ExecuteDeserializeHandler(Ctx, PredPair.Value);
	}

	FFieldVariant& Property = Ctx.TopProperty();
	FDcDeserializeDelegate* HandlerPtr;
	if (Property.IsUObject())
	{
		UObject* Object = CastChecked<UObject>(Property.ToUObjectUnsafe());
		check(IsValid(Object));
		UClass* Class = Object->GetClass();
		HandlerPtr = Self->UClassDeserializerMap.Find(Class);
		if (HandlerPtr == nullptr)
			return DC_FAIL(DcDSerDe, NoMatchingHandler)
				<< Ctx.TopProperty().GetFName() << Class->GetFName();
	}
	else
	{
		FField* Field = Property.ToFieldUnsafe();
		check(Field->IsValidLowLevel());
		FFieldClass* FieldClass = Field->GetClass();
		HandlerPtr = Self->FieldClassDeserializerMap.Find(FieldClass);
		if (HandlerPtr == nullptr)
			return DC_FAIL(DcDSerDe, NoMatchingHandler)
				<< Ctx.TopProperty().GetFName() << FieldClass->GetFName();
	}

	return ExecuteDeserializeHandler(Ctx, *HandlerPtr);
}

static void AmendDiagnostic(FDcDiagnostic& Diag, FDcDeserializeContext& Ctx)
{
	if (Diag.Highlights.IndexOfByPredicate([&Ctx](auto& Highlight){
		return Highlight.Owner == Ctx.Deserializer;}) != INDEX_NONE)
		return;

	FDcDiagnosticHighlight Highlight(Ctx.Deserializer, TEXT("DcSerializer"));
	TStringBuilder<1024> Sb;
	{
		Sb.Append(TEXT("\n### Properties:"));
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

	{
		Sb.Append(TEXT("\n### Objects:"));
		int Num = Ctx.Objects.Num();
		if (Num)
		{
			Sb.Appendf(TEXT(" (%d) :"), Num);
			for (UObject* Obj : Ctx.Objects)
			{
				Sb.Append(TEXT("\n  - "));
				Sb.Append(DcDiagnosticUtils::SafeObjectName(Obj));
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

}	// namespace DcDeserializerDetails

FDcResult FDcDeserializer::Deserialize(FDcDeserializeContext& Ctx)
{
	auto _AmendDiag = [](FDcDeserializeContext& Ctx)
	{
		FDcDiagnostic& Diag = DcEnv().GetLastDiag();
		DcDiagnosticUtils::AmendDiagnostic(Diag, Ctx.Reader, Ctx.Writer);
		DcDeserializerDetails::AmendDiagnostic(Diag, Ctx);
	};

	using ECtxState = FDcDeserializeContext::EState;
	if (Ctx.State == ECtxState::Uninitialized)
	{
		return DC_FAIL(DcDSerDe, NotPrepared);
	}
	else if (Ctx.State == ECtxState::Ready)
	{
		Ctx.State = FDcDeserializeContext::EState::DeserializeInProgress;

		ON_SCOPE_EXIT
		{
			Ctx.State = ECtxState::DeserializeEnded;
		};

		FDcResult Result = DcDeserializerDetails::DeserializeBody(this, Ctx);
		if (!Result.Ok()) _AmendDiag(Ctx);

		return Result;
	}
	else if (Ctx.State == FDcDeserializeContext::EState::DeserializeInProgress)
	{
		FDcResult Result = DcDeserializerDetails::DeserializeBody(this, Ctx);
		if (!Result.Ok()) _AmendDiag(Ctx);

		return Result;
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

