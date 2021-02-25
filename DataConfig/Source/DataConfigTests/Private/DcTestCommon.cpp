#include "DcTestCommon.h"

#include "DataConfig/Misc/DcTypeUtils.h"

static_assert(DcTypeUtils::TIsUClass<UClass>::Value, "test TIsUClass");
static_assert(!DcTypeUtils::TIsUClass<FDcReader>::Value, "test TIsUClass");

static_assert(!DcTypeUtils::TIsUStruct<UClass>::Value, "test TIsUStruct");
static_assert(!DcTypeUtils::TIsUStruct<FDcReader>::Value, "test TIsUStruct");
