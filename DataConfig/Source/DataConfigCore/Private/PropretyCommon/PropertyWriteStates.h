#pragma once

#include "UObject/UnrealType.h"

namespace DataConfig {

enum class EPropertyWriteType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
};


enum class EDataEntry;

struct FBaseWriteState
{
	virtual EPropertyWriteType GetType() = 0;

	virtual FResult Peek(EDataEntry Next);
	virtual FResult WriteName(const FName& Value);
	virtual FResult WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum);

	template<typename T>
	T* As();

};

template<typename T>
T* FBaseWriteState::As()
{
	if (GetType() == T::ID)
		return (T*)this;
	else
		return nullptr;
}

struct FWriteStateNil : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::Nil;

	EPropertyWriteType GetType() override;
};

struct FWriteStateStruct : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::StructProperty;

	void* StructPtr;
	UScriptStruct* StructClass;
	UProperty* Property;

	enum class EState
	{
		ExpectRoot,
		ExpectKeyOrEnd,
		ExpectValue,
		Ended,
	};
	EState State;

	FWriteStateStruct(void* InStructPtr, UScriptStruct* InStructClass)
	{
		StructPtr = InStructPtr;
		StructClass = InStructClass;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EPropertyWriteType GetType() override;
	FResult Peek(EDataEntry Next) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum) override;

};




} // namespace DataConfig


