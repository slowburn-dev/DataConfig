#include <HAL/Platform.h>

template<class CharType>
FString TDcSourceRef<CharType>::ToString()
{
	//	this constructor already accept both char types
	return FString(Num, SourceBuffer->Buffer + Begin);
}


