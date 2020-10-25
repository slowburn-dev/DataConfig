
template<class CharType>
FString TDcSourceRef<CharType>::ToString()
{
	//	this constructor already accept both char types
	return FString(Num, SourceBuffer->Buffer + Begin);
}

template<class CharType>
bool TDcSourceRef<CharType>::IsValid() const
{
	return SourceBuffer != nullptr;
}

template<class CharType>
void TDcSourceRef<CharType>::Reset()
{
	SourceBuffer = nullptr;
}
