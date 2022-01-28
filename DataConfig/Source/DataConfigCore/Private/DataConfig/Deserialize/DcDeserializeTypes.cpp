#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"

FDcResult FDcDeserializeContext::Prepare()
{
	DC_TRY(DcExpect(State == EState::Uninitialized, [&]{ 
		return DC_FAIL(DcDSerDe, ContextInvalidState) << (int)State;
	}));

	DC_TRY(DcExpect(Reader != nullptr, []{
		return DC_FAIL(DcDSerDe, ContextReaderNotFound);
	}));

	DC_TRY(DcExpect(Writer != nullptr, []{
		return DC_FAIL(DcDSerDe, ContextWriterNotFound);
	}));

	DC_TRY(DcExpect(Deserializer != nullptr, []{
		return DC_FAIL(DcDSerDe, ContextDeserializerNotFound);
	}));

	//	for convenience if no top properties are provide, we peek one from property writer.
	if (Properties.Num() == 0)
	{
		FFieldVariant RootProperty;
		DC_TRY(Writer->PeekWriteProperty(&RootProperty));
		Properties.Push(RootProperty);
	}
	else if (Properties.Num() == 1)
	{
		// pass
	}
	else
	{
		return DC_FAIL(DcDSerDe, ContextExpectOneProperty) << Properties.Num();
	}

	State = EState::Ready;
	return DcOk();
}
