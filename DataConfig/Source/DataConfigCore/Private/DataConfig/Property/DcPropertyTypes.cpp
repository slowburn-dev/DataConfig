#include "DataConfig/Property/DcPropertyTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"

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
