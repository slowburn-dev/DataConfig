#include "DataConfig/Property/DcPropertyDatum.h"

const FDcPropertyDatum FDcPropertyDatum::NONE = FDcPropertyDatum();

FDcPropertyDatum::FDcPropertyDatum()
{
	Property = FFieldVariant();
	DataPtr = nullptr;
}

FDcPropertyDatum::FDcPropertyDatum(FField* InProperty, void* InDataPtr)
{
	Property = InProperty;
	DataPtr = InDataPtr;
}

FDcPropertyDatum::FDcPropertyDatum(UObject* ClassObject)
{
	check(ClassObject);
	Property = ClassObject->GetClass();
	DataPtr = ClassObject;
}

FDcPropertyDatum::FDcPropertyDatum(UClass* Class, UObject* ClassObject)
{
	check(Class && ClassObject);
	Property = Class;
	DataPtr = ClassObject;
}

FDcPropertyDatum::FDcPropertyDatum(UScriptStruct* StructClass, void* StructPtr)
{
	check(StructClass);
	Property = StructClass;
	DataPtr = StructPtr;
}

