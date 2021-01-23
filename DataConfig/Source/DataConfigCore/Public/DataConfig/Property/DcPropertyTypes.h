#pragma once

#include "DataConfig/DcTypes.h"
#include "UObject/UnrealType.h"

using FDcProcessPropertyPredicateSignature = bool(*)(FProperty* Property);
DECLARE_DELEGATE_RetVal_OneParam(bool, FDcProcessPropertyPredicateDelegate, FProperty*);

using FDcExpandObjectPredicateSignature = bool(*)(FObjectProperty* ObjectProperty);
DECLARE_DELEGATE_RetVal_OneParam(bool, FDcExpandObjectPredicateDelegate, FObjectProperty*);

struct DATACONFIGCORE_API FDcPropertyConfig
{
	FDcProcessPropertyPredicateDelegate ProcessPropertyPredicate;
	FDcExpandObjectPredicateDelegate ExpandObjectPredicate;

	static FDcPropertyConfig MakeDefault();

};


