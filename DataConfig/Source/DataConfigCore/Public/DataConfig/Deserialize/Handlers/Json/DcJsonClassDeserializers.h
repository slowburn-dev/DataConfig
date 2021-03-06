#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcJsonHandlers {

DATACONFIGCORE_API FDcResult TryReadObjectReference(FObjectPropertyBase* ObjectProperty, FDcReader* Reader, UObject*& OutObject);

DATACONFIGCORE_API FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx);

struct DATACONFIGCORE_API FObjectReferenceHandlerGenerator
{
	using ObjectReaderFuncType = TFunction<FDcResult(FObjectPropertyBase*, FDcReader*, UObject*&)>;
	FObjectReferenceHandlerGenerator(ObjectReaderFuncType InObjectReader)
		: FuncObjectReader(InObjectReader)
	{}

	ObjectReaderFuncType FuncObjectReader;

	FDcDeserializeDelegate MakeObjectReferenceHandler();
	FDcDeserializeDelegate MakeSoftObjectReferenceHandler();
	FDcDeserializeDelegate MakeWeakObjectReferenceHandler();
	FDcDeserializeDelegate MakeLazyObjectReferenceHandler();
};

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx);

}	// namespace DcHandlers
