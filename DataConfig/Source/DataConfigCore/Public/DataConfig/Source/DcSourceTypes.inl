
template<class CharType>
FString TDcSourceRef<CharType>::CharsToString() const
{
	//	! note that this would convert non ascii UTF8 code points to `?`
	return IsValid()
		? FString(Num, Buffer->Buffer + Begin)
		: TEXT("<INVALIDREF>");
}

template<class CharType>
bool TDcSourceRef<CharType>::IsValid() const
{
	return Buffer != nullptr
		&& Begin >= 0
		&& Begin + Num <= Buffer->Num;
}

template<class CharType>
const CharType* TDcSourceRef<CharType>::GetBeginPtr() const
{
	if (!IsValid())
	{
		return nullptr;
	}
	else
	{
		return Buffer->Buffer + Begin;
	}
}

template<class CharType>
void TDcSourceRef<CharType>::Reset()
{
	Buffer = nullptr;
}
