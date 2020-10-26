
template<class CharType>
FString TDcSourceRef<CharType>::ToString()
{
	//	this constructor already accept both char types
	return FString(Num, Buffer->Buffer + Begin);
}

template<class CharType>
bool TDcSourceRef<CharType>::IsValid() const
{
	return Buffer != nullptr
		&& Begin >= 0
		&& Num <= Buffer->Num;
}

template<class CharType>
void TDcSourceRef<CharType>::Reset()
{
	Buffer = nullptr;
}
