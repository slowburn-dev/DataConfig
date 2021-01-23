#include "DataConfig/Property/DcPropertyTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/DcEnv.h"

FDcPropertyConfig FDcPropertyConfig::MakeDefault()
{
	static FDcPropertyConfig _DEFAULT = [](){
		FDcPropertyConfig Ret;

		Ret.ProcessPropertyPredicate = FDcProcessPropertyPredicateDelegate::CreateStatic(DcPropertyUtils::IsEffectiveProperty);
		Ret.ExpandObjectPredicate = FDcExpandObjectPredicateDelegate::CreateStatic(DcPropertyUtils::IsSubObjectProperty);

		return Ret;
	}();

	return _DEFAULT;
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

FProperty* FDcPropertyConfig::NextProcessPropertyByName(FProperty* InProperty, const FName& Name)
{
	if (InProperty == nullptr)
		return nullptr;

	for (FProperty* Property = InProperty; Property; Property = Property->PropertyLinkNext)
	{
		if (Property->GetFName() == Name
			&& ShouldProcessProperty(Property))
		{
			return Property;
		}
	}

	return nullptr;
}
