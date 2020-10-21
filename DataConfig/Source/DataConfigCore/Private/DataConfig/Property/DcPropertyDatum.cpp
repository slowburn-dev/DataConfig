#include "DataConfig/Property/DcPropertyDatum.h"

const FDcPropertyDatum FDcPropertyDatum::NONE = FDcPropertyDatum();

FDcPropertyDatum::FDcPropertyDatum()
{
	Property = nullptr;
	DataPtr = nullptr;
}

FDcPropertyDatum::FDcPropertyDatum(UField* InProperty, void* InDataPtr)
{
	Property = InProperty;
	DataPtr = InDataPtr;
}

FDcPropertyDatum::FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr)
{
	check(StructClass);
	Property = StructClass;
	DataPtr = StructPtr;
}

FDcPropertyDatum::FDcPropertyDatum(UObject* ClassObject)
{
	check(ClassObject);
	Property = ClassObject->GetClass();
	DataPtr = ClassObject;
}

