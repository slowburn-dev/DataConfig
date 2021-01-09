#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"

FDcScopedProperty::FDcScopedProperty(FDcDeserializeContext& InCtx)
	: Ctx(InCtx)
{}

FDcResult FDcScopedProperty::PushProperty()
{
	DC_TRY(Ctx.Writer->PeekWriteProperty(&Property));
	Ctx.Properties.Push(Property);
	return DcOk();
}

FDcScopedProperty::~FDcScopedProperty()
{
	if (Property != nullptr)
	{
		check(Property == Ctx.Properties.Top());
		Ctx.Properties.Pop();
	}
}

FDcScopedObject::FDcScopedObject(FDcDeserializeContext& InCtx, UObject* InObject)
	: Object(InObject)
	, Ctx(InCtx)
{
	check(Object);
	Ctx.Objects.Push(Object);
}

FDcScopedObject::~FDcScopedObject()
{
	UObject* Popped = Ctx.Objects.Pop();
	check(Popped == Object);
}


FDcResult FDcDeserializeContext::Prepare()
{
	DC_TRY(DcExpect(State == EState::Uninitialized, [&]{ 
		return DC_FAIL(DcDDeserialize, ContextInvalidState) << (int)State;
	}));

	DC_TRY(DcExpect(Reader != nullptr, []{
		return DC_FAIL(DcDDeserialize, ContextReaderNotFound);
	}));

	DC_TRY(DcExpect(Writer != nullptr, []{
		return DC_FAIL(DcDDeserialize, ContextWriterNotFound);
	}));

	DC_TRY(DcExpect(Deserializer != nullptr, []{
		return DC_FAIL(DcDDeserialize, ContextDeserializerNotFound);
	}));

	DC_TRY(DcExpect(Properties.Num() == 1, [&]{
		return DC_FAIL(DcDDeserialize, ContextExpectOneProperty) << Properties.Num();
	}));

	State = EState::Ready;
	return DcOk();
}
