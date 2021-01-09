#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Property/DcPropertyWriter.h"

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
	//	wraps these into checks
	check(State == EState::Uninitialized);
	check(Reader != nullptr);
	check(Writer != nullptr);
	check(Deserializer != nullptr);
	check(Properties.Num() == 1);

	State = EState::Ready;

	return DcOk();
}
