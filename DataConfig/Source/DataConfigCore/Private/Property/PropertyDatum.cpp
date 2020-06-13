#include "Property/PropertyDatum.h"

namespace DataConfig {

const FPropertyDatum FPropertyDatum::NONE = FPropertyDatum();

FPropertyDatum::FPropertyDatum()
{
	Property = nullptr;
	DataPtr = nullptr;
}

FPropertyDatum::FPropertyDatum(UField* InProperty, void* InDataPtr)
{
	Property = InProperty;
	DataPtr = InDataPtr;
}

FPropertyDatum::FPropertyDatum(UScriptStruct* StructClass, void* StructPtr)
{
	check(StructClass);
	Property = StructClass;
	DataPtr = StructPtr;
}

FPropertyDatum::FPropertyDatum(UObject* ClassObject)
{
	check(ClassObject);
	Property = ClassObject->GetClass();
	DataPtr = ClassObject;
}

} // namespace DataConfig

