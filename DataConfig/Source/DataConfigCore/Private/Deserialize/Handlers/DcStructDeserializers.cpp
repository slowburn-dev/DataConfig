#include "Deserialize/Handlers/DcStructDeserializers.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "Deserialize/DcDeserializer.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig
{


static FName GetStructName(UField* Property)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		return StructProperty->Struct->GetFName();
	}
	else if (UScriptStruct* StructClass = Cast<UScriptStruct>(Property))
	{
		return StructClass->GetFName();
	}
	else
	{
		return FName();
	}
}

FResult DATACONFIGCORE_API StructRootDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	EDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDataEntry::StructRoot).Ok();
	if (!bRootPeekPass
		|| !bWritePass)
	{
		return OkWithCanNotProcess(OutRet);
	}

	if (Next == EDataEntry::MapRoot)
	{
		TRY(Ctx.Reader->ReadMapRoot(nullptr));
		TRY(Ctx.Writer->WriteStructRoot(GetStructName(Ctx.TopProperty())));

		EDataEntry CurPeek = Ctx.Reader->Peek();
		while (CurPeek != EDataEntry::MapEnd)
		{
			if (CurPeek == EDataEntry::Name)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FName Value;
				TRY(Ctx.Reader->ReadName(&Value, nullptr));
				TRY(Ctx.Writer->WriteName(Value));
			}
			else if (CurPeek == EDataEntry::String)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FString Value;
				TRY(Ctx.Reader->ReadString(&Value, nullptr));
				TRY(Ctx.Writer->WriteName(FName(*Value)));
			}
			else
			{
				return Fail(EErrorCode::DeserializeTypeNotMatch);
			}

			FScopedProperty ScopedValueProperty(Ctx);
			TRY(ScopedValueProperty.PushProperty());
			TRY(Ctx.Deserializer->Deserialize(Ctx));

			CurPeek = Ctx.Reader->Peek();
		}

		TRY(Ctx.Reader->ReadMapEnd(nullptr));
		TRY(Ctx.Writer->WriteStructEnd(GetStructName(Ctx.TopProperty())));

		return OkWithProcessed(OutRet);
	}
	else 
	{
		return Fail(EErrorCode::UnknownError);
	}
}

} // namespace DataConfig
