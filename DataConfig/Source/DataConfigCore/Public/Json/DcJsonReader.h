#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Reader/DcReader.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FJsonReader : public FReader, private FNoncopyable
{
	using TCharType = TCHAR;

	FJsonReader() = default;
	FJsonReader(const FString* InStrPtr);

	void SetNewString(const FString* InStrPtr);

	enum class EState
	{
		Unitialized,
		InitializedWithStr,
		FinishedStr,
		Invalid,
	};
	EState State = EState::Unitialized;

	const FString* StrPtr = nullptr;
	int32 Cur = 0;

	EDataEntry Peek() override;

	FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FResult ReadMapEnd(FContextStorage* CtxPtr) override;
	
	bool IsAtEnd();
	TCharType ReadChar();
	void PutBack();

	FResult ReadCharExpect(TCharType Expect, EErrorCode ErrCode);

};






} // namespace DataConfig









