#include "DataConfig/Property/DcPropertyTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/DcEnv.h"
#include "UObject/PropertyAccessUtil.h"

namespace DcPropertyTypesDetails
{

static bool _DefaultProcessPropertyPredicate(FProperty* Property)
{
#if WITH_EDITORONLY_DATA
	return DcPropertyUtils::IsEffectiveProperty(Property)
		&& !Property->HasMetaData(DcPropertyUtils::DC_META_SKIP);
#else
	return DcPropertyUtils::IsEffectiveProperty(Property);
#endif // WITH_EDITORONLY_DATA
}

} // namespace DcPropertyTypesDetails

FDcPropertyConfig FDcPropertyConfig::MakeDefault()
{
	static FDcPropertyConfig _DEFAULT = [](){
		FDcPropertyConfig Ret;

		Ret.ProcessPropertyPredicate = FDcProcessPropertyPredicateDelegate::CreateStatic(DcPropertyTypesDetails::_DefaultProcessPropertyPredicate);
		Ret.ExpandObjectPredicate = FDcExpandObjectPredicateDelegate::CreateStatic(DcPropertyUtils::IsSubObjectProperty);

		return Ret;
	}();

	return _DEFAULT;
}

FDcPropertyConfig FDcPropertyConfig::MakeNoExpandObject()
{
	static FDcPropertyConfig _NO_EXPAND = []()
	{
		FDcPropertyConfig Ret;

		Ret.ProcessPropertyPredicate = FDcProcessPropertyPredicateDelegate::CreateStatic(DcPropertyTypesDetails::_DefaultProcessPropertyPredicate);
		Ret.ExpandObjectPredicate = FDcExpandObjectPredicateDelegate::CreateLambda([](FObjectProperty*){ return false; });

		return Ret;
	}();

	return _NO_EXPAND;
}

FDcResult FDcPropertyConfig::Prepare()
{
	if (!ProcessPropertyPredicate.IsBound()) return DC_FAIL(DcDCommon, StaleDelegate);
	if (!ExpandObjectPredicate.IsBound()) return DC_FAIL(DcDCommon, StaleDelegate);

	return DcOk();
}

bool FDcPropertyConfig::ShouldProcessProperty(FProperty* Property)
{
	return ProcessPropertyPredicate.Execute(Property);
}

bool FDcPropertyConfig::ShouldExpandObject(FObjectProperty* ObjectProperty)
{
	return ExpandObjectPredicate.Execute(ObjectProperty);
}

FProperty* FDcPropertyConfig::NextProcessProperty(FProperty* Property)
{
	while (true)
	{
		if (Property == nullptr)
			return nullptr;

		Property = Property->PropertyLinkNext;

		if (Property == nullptr)
			return nullptr;

		if (ShouldProcessProperty(Property))
			return Property;
	}

	checkNoEntry();
	return nullptr;
}

FProperty* FDcPropertyConfig::FirstProcessProperty(FProperty* Property)
{
	if (Property == nullptr)
		return nullptr;

	return ShouldProcessProperty(Property)
		? Property
		: NextProcessProperty(Property);
}

FProperty* FDcPropertyConfig::NextProcessPropertyByName(UStruct* Struct, FProperty* InProperty, const FName& Name)
{
	if (InProperty
		&& InProperty->PropertyLinkNext
		&& InProperty->PropertyLinkNext->GetFName() == Name)
	{
		//	fast path
		return InProperty->PropertyLinkNext;
	}
	else
	{
		return FindProcessPropertyByName(Struct, Name);
	}
}

FProperty* FDcPropertyConfig::FindProcessPropertyByName(UStruct* Struct, const FName& Name)
{
	FProperty* Target = DcPropertyUtils::FindEffectivePropertyByName(Struct, Name);
	if (!Target)
		return nullptr;
	
	return ShouldProcessProperty(Target)
		? Target
		: nullptr;
}

