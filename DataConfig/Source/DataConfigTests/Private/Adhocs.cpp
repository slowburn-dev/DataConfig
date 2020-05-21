#include "Adhocs.h"
#include "Reader/PropertyReader.h"
#include "UObject/UnrealType.h"
#include "Reader/ReaderErrorCodes.h"

using namespace DataConfig;

struct FScafVisitor : public FVisitor
{
	FVisitResult VisitBool(bool Value) override
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Visiting a bool: %d"), Value);
		return Ok();
	}

	FVisitResult VisitName(FName Name) override
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Visiting a name: %s"), *Name.ToString());
		return Ok();
	}

	FVisitResult VisitString(FString Str) override
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("Visiting a str: %s"), *Str);
		return Ok();
	}

	FVisitResult VisitMap(FMapAccess& MapAccess) override
	{
		while (true)
		{
			bool bHasPending;
			FVisitResult Ret;

			Ret = MapAccess.HasPending(bHasPending);
			if (!bHasPending) return Ret;
			if (!Ret.Ok()) return Ret;
			Ret = MapAccess.ReadKey(*this);
			if (!Ret.Ok()) return Ret;
			Ret = MapAccess.ReadValue(*this);
			if (!Ret.Ok()) return Ret;
			Ret = MapAccess.Next();
			if (!Ret.Ok()) return Ret;
		}

		return Ok();
	}
};

struct FDatum
{
	TWeakObjectPtr<UField> Struct;
	void* DataPtr = nullptr;

	bool IsValid()
	{
		return Struct.IsValid()
			&& DataPtr != nullptr;
	}
};

struct FGetNameVisitor : public FVisitor
{
	FName Name;

	FVisitResult VisitName(FName aName) override
	{
		this->Name = aName;
		return Ok();
	}
};


static UProperty* GetPropertyByName(UStruct* Struct, const FName& Name)
{
	for (UProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (Name == Property->GetFName())
		{
			return Property;
		}
	}

	return nullptr;
}

//	OR this can be pushed as the current reader
struct FWritePropertyVisitor : public FVisitor
{
	//	actually this doesn't needs to be a WeakObjectPtr as 
	//	it's not going to be deleted during the visit
	TWeakObjectPtr<UField> Struct;	
	void* DataPtr = nullptr;

	FVisitResult VisitBool(bool Value) override {
		if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Struct.Get())) {
			BoolProperty->SetPropertyValue(DataPtr, Value);
			return Ok();
		}
		else
		{
			return Fail(EReaderErrorCode::ExpectBoolFail);
		}
	}

	FVisitResult VisitName(FName Value) override {
		if (UNameProperty* NameProperty = Cast<UNameProperty>(Struct.Get())) {
			NameProperty->SetPropertyValue(DataPtr, Value);
			return Ok();
		}
		else
		{
			return Fail(EReaderErrorCode::ExpectNameFail);
		}
	}

	FVisitResult VisitString(FString Value) override {
		if (UStrProperty* StrProperty = Cast<UStrProperty>(Struct.Get()))
		{
			StrProperty->SetPropertyValue(DataPtr, Value);
			return Ok();
		}
		else
		{
			return Fail(EReaderErrorCode::ExpectStringFail);
		}
	}

};

//	TODO custom visitor that takes a context and Datum in
struct FConstructVisitor : public FVisitor
{
	FDatum Datum;

	FConstructVisitor(FDatum Datum)
		: Datum(Datum)
	{
		//	TODO expecting 
	}

	FVisitResult VisitMap(FMapAccess& MapAccess) override
	{
		UScriptStruct* Struct = CastChecked<UScriptStruct>(Datum.Struct.Get());
		void* StructPtr = Datum.DataPtr;

		while (true)
		{
			bool bHasPending;
			FVisitResult Ret;

			Ret = MapAccess.HasPending(bHasPending);
			if (!bHasPending) return Ret;
			if (!Ret.Ok()) return Ret;

			//	damn how do i construct this?
			//	the best case is to construct a value out of this
			//	setting state would work, it's just too confusing
			FGetNameVisitor NameGetter;
			Ret = MapAccess.ReadKey(NameGetter);	
			if (!Ret.Ok()) return Ret;

			//	Get Property By Name
			UProperty* Property = GetPropertyByName(Struct, NameGetter.Name);
			if (!Property)
				return Fail(EReaderErrorCode::UnknownError);

			FWritePropertyVisitor Writer;
			Writer.Struct = Property;
			Writer.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

			Ret = MapAccess.ReadValue(Writer);
			if (!Ret.Ok()) return Ret;
			Ret = MapAccess.Next();
			if (!Ret.Ok()) return Ret;
		}

		return Ok();
	}

};

void PropertyReaderScaffolding()
{
	FScafVisitor Visitor;

	FTestStruct_Alpha StructAlpha{};
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));

	FPropertyReader Reader(
		(void*)&StructAlpha,
		FTestStruct_Alpha::StaticStruct()
	);

	Reader.ReadAny(Visitor);

	return;
}
void PropertyVisitorRoundtrip()
{
	FTestStruct_Alpha StructAlpha{};
	StructAlpha.ABool = true;
	StructAlpha.AName = FName(TEXT("ALPHA"));
	StructAlpha.AStr = FString(TEXT("A L P H A"));

	FTestStruct_Alpha OutAlpha;

	{
		FDatum Datum;
		Datum.Struct = FTestStruct_Alpha::StaticStruct();
		Datum.DataPtr = &OutAlpha;

		FConstructVisitor OutVisitor(Datum);

		FPropertyReader Reader(
			(void*)&StructAlpha,
			FTestStruct_Alpha::StaticStruct()
		);

		Reader.ReadAny(OutVisitor);
	}

	//	then print
	{
		UE_LOG(LogDataConfigCore, Display, TEXT("--------------------------------------"));
		FScafVisitor Visitor;
		FPropertyReader Reader(
			(void*)&OutAlpha,
			FTestStruct_Alpha::StaticStruct()
		);

		Reader.ReadAny(Visitor);
	}

	return;
}

