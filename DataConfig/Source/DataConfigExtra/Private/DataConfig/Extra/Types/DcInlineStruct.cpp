#include "DataConfig/Extra/Types/DcInlineStruct.h"

template struct DATACONFIGEXTRA_API TDcInlineStructBase<64>;
template struct DATACONFIGEXTRA_API TDcInlineStructBase<128>;
template struct DATACONFIGEXTRA_API TDcInlineStructBase<256>;
template struct DATACONFIGEXTRA_API TDcInlineStructBase<512>;

//	compile hack for c++14 constexpr
template<int32 BUFLEN> constexpr int32 TDcInlineStructBase<BUFLEN>::_BUFLEN;
template<int32 BUFLEN> constexpr int32 TDcInlineStructBase<BUFLEN>::_CLASS_OFFSET;


