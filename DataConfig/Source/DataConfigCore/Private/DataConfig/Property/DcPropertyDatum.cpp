#include "DataConfig/Property/DcPropertyDatum.h"

const FDcPropertyDatum FDcPropertyDatum::NONE = FDcPropertyDatum();

FDcPropertyDatum::FDcPropertyDatum()
{
	Property = FFieldVariant();
	DataPtr = nullptr;
}

FDcPropertyDatum::FDcPropertyDatum(FField* InProperty, void* InDataPtr)
{
	Property = FFieldVariant(InProperty);
	DataPtr = InDataPtr;
}

FDcPropertyDatum::FDcPropertyDatum(UObject* ClassObject)
{
	check(ClassObject);
	Property = FFieldVariant(ClassObject->GetClass());
	DataPtr = ClassObject;
}

FDcPropertyDatum::FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr)
{
	check(StructClass);
	Property = FFieldVariant(StructClass);
	DataPtr = StructPtr;
}
