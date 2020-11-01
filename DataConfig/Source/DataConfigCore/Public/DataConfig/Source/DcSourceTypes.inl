
template<class CharType>
FString TDcSourceRef<CharType>::ToString() const
{
	//	this constructor already accept both char types
	return FString(Num, Buffer->Buffer + Begin);
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
