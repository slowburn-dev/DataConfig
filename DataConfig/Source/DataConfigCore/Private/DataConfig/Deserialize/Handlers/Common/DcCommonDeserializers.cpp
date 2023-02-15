#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/SerDe/DcSerDeUtils.inl"

namespace DcCommonHandlers {

void AddNumericPipeDirectHandlers(FDcDeserializer& Deserializer)
{
	Deserializer.AddDirectHandler(FInt8Property::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeInt8Deserialize));
	Deserializer.AddDirectHandler(FInt16Property::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeInt16Deserialize));
	Deserializer.AddDirectHandler(FIntProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeInt32Deserialize));
	Deserializer.AddDirectHandler(FInt64Property::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeInt64Deserialize));

	Deserializer.AddDirectHandler(FByteProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeUInt8Deserialize));
	Deserializer.AddDirectHandler(FUInt16Property::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeUInt16Deserialize));
	Deserializer.AddDirectHandler(FUInt32Property::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeUInt32Deserialize));
	Deserializer.AddDirectHandler(FUInt64Property::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeUInt64Deserialize));

	Deserializer.AddDirectHandler(FFloatProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeFloatDeserialize));
	Deserializer.AddDirectHandler(FDoubleProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeDoubleDeserialize));
}

EDcDeserializePredicateResult PredicateIsScalarArrayProperty(FDcDeserializeContext& Ctx)
{
	FProperty* Prop = CastField<FProperty>(Ctx.TopProperty().ToField());
	return Prop && Prop->ArrayDim > 1 && !Ctx.Writer->IsWritingScalarArrayItem()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerPipeScalarDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeScalar(Ctx);
}

FDcResult HandlerPipeBoolDeserialize(FDcDeserializeContext& Ctx) { return DcPipe_Bool(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeNameDeserialize(FDcDeserializeContext& Ctx) { return DcPipe_Name(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeStringDeserialize(FDcDeserializeContext& Ctx) { return DcPipe_String(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeTextDeserialize(FDcDeserializeContext& Ctx) { return DcPipe_Text(Ctx.Reader, Ctx.Writer);}

FDcResult HandlerPipeFloatDeserialize(FDcDeserializeContext& Ctx) { return DcPipe_Float(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeDoubleDeserialize(FDcDeserializeContext& Ctx) { return DcPipe_Double(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeInt8Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_Int8(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeInt16Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_Int16(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeInt32Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_Int32(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeInt64Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_Int64(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeUInt8Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_UInt8(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeUInt16Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_UInt16(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeUInt32Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_UInt32(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeUInt64Deserialize(FDcDeserializeContext& Ctx) { return DcPipe_UInt64(Ctx.Reader, Ctx.Writer); }

EDcDeserializePredicateResult PredicateIsEnumProperty(FDcDeserializeContext& Ctx)
{
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(Ctx.TopProperty(), Enum, UnderlyingProperty);
	
	return bIsEnum 
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerStringToEnumDeserialize(FDcDeserializeContext& Ctx)
{
	FFieldVariant& TopProperty = Ctx.TopProperty();
	UEnum* Enum;
	FNumericProperty* UnderlyingProperty;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(TopProperty, Enum, UnderlyingProperty);

	if (!bIsEnum)
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
			<< TEXT("EnumProperty")  << TEXT("<NumericProperty with Enum>")
			<< TopProperty.GetFName() << TopProperty.GetClassName();

	if (Enum == nullptr)
	{
		//	fallback to read numeric
		FDcEnumData EnumData;
		DC_TRY(Ctx.Reader->ReadUInt64(&EnumData.Unsigned64));
		DC_TRY(Ctx.Writer->WriteEnum(EnumData));

		return DcOk();
	}

	bool bIsBitFlags;

#if WITH_EDITORONLY_DATA

	#if WITH_METADATA
	bIsBitFlags = Enum->HasMetaData(TEXT("Bitflags"));
	#else
	//	Program target is missing `UEnum::HasMetaData`
	bIsBitFlags = ((UField*)Enum)->HasMetaData(TEXT("Bitflags"));
	#endif

#else // WITH_EDITORONLY_DATA
	bIsBitFlags = false;
#endif // WITH_EDITORONLY_DATA

	
	if (!bIsBitFlags)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

		FName ValueName(Enum->GenerateFullEnumName(*Value));
		if (!Enum->IsValidEnumName(ValueName))
			return DC_FAIL(DcDReadWrite, EnumNameNotFound) << Enum->GetFName() << Value;

		FDcEnumData EnumData;
		EnumData.Signed64 = Enum->GetValueByName(ValueName);

		DC_TRY(Ctx.Writer->WriteEnum(EnumData));
		return DcOk();
	}
	else
	{
		FDcEnumData EnumData;
		EnumData.Signed64 = 0;

		DC_TRY(Ctx.Reader->ReadArrayRoot());
		while (true)
		{
			EDcDataEntry Next;
			DC_TRY(Ctx.Reader->PeekRead(&Next));
			if (Next == EDcDataEntry::ArrayEnd)
				break;

			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));

			FName ValueName(Enum->GenerateFullEnumName(*Value));
			if (!Enum->IsValidEnumName(ValueName))
				return DC_FAIL(DcDReadWrite, EnumNameNotFound) << Enum->GetFName() << Value;

			EnumData.Signed64 |= Enum->GetValueByName(ValueName);
		}
		DC_TRY(Ctx.Reader->ReadArrayEnd());
		DC_TRY(Ctx.Writer->WriteEnum(EnumData));

		return DcOk();
	}
}

FDcResult HandlerStringToFieldPathDeserialize(FDcDeserializeContext& Ctx)
{
	FString Str;
	DC_TRY(Ctx.Reader->ReadString(&Str));

	FFieldPath Value;
	Value.Generate(*Str);
	DC_TRY(Ctx.Writer->WriteFieldPath(Value));
	
	return DcOk();
}

FDcResult HandlerMapToStructDeserialize(FDcDeserializeContext& Ctx)
{
	FDcStructAccess Access;
	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
			break;

		FName FieldName;
		DC_TRY(Ctx.Reader->ReadName(&FieldName));
		DC_TRY(Ctx.Writer->WriteName(FieldName));

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteStructEnd());

	return DcOk();
}

FDcResult HandlerMapToClassDeserialize(FDcDeserializeContext& Ctx)
{
	FDcClassAccess Access{FDcClassAccess::EControl::ExpandObject};
	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteClassRootAccess(Access));

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
		{
			break;
		}
		else if (CurPeek == EDcDataEntry::String)
		{
			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));
			if (DcSerDeUtils::IsMeta(Value))
			{
				//	skip next object
				DC_TRY(DcSerDeUtils::ReadNoopConsumeValue(Ctx.Reader));
				continue;
			}
			else
			{
				DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
			}
		}
		else
		{
			return DC_FAIL(DcDSerDe, DataEntryMismatch2)
				<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
		}

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteClassEndAccess(Access));
	return DcOk();
}

FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		EDcDataEntry::ArrayEnd,
		&DcDeserializeUtils::RecursiveDeserialize,
		&FDcReader::ReadArrayRoot,
		&FDcReader::ReadArrayEnd,
		&FDcPropertyWriter::WriteArrayRoot,
		&FDcPropertyWriter::WriteArrayEnd
	>(Ctx);
}

FDcResult HandlerArrayToSetDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		EDcDataEntry::ArrayEnd,
		&DcDeserializeUtils::RecursiveDeserialize,
		&FDcReader::ReadArrayRoot,
		&FDcReader::ReadArrayEnd,
		&FDcPropertyWriter::WriteSetRoot,
		&FDcPropertyWriter::WriteSetEnd
	>(Ctx);
}

FDcResult HandlerMapOrArrayOfKeyValueDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next == EDcDataEntry::MapRoot)
	{
		return DcHandlerPipeMap<
			FDcDeserializeContext,
			FDcReader,
			FDcPropertyWriter,
			&DcDeserializeUtils::RecursiveDeserialize
		>(Ctx);
	}
	else if (Next == EDcDataEntry::ArrayRoot)
	{
		DC_TRY(Ctx.Reader->ReadArrayRoot());
		DC_TRY(Ctx.Writer->WriteMapRoot());

		EDcDataEntry Cur;
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&Cur));
			if (Cur == EDcDataEntry::ArrayEnd)
				break;

			//	{
			//		"$key" : <arbitrary key>,
			//		"$value" : <arbitrary value>
			//	}

			FString MetaKey;
			DC_TRY(Ctx.Reader->ReadMapRoot());
			DC_TRY(Ctx.Reader->ReadString(&MetaKey));
			DC_TRY(DcSerDeUtils::ExpectMetaKey(MetaKey, TEXT("$key")));

			DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));

			DC_TRY(Ctx.Reader->ReadString(&MetaKey));
			DC_TRY(DcSerDeUtils::ExpectMetaKey(MetaKey, TEXT("$value")));

			DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
			DC_TRY(Ctx.Reader->ReadMapEnd());
		}

		DC_TRY(Ctx.Reader->ReadArrayEnd());
		DC_TRY(Ctx.Writer->WriteMapEnd());
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch2)
			<< EDcDataEntry::MapRoot << EDcDataEntry::ArrayRoot << Next;
	}

	return DcOk();
}


} // namespace DcCommonHandlers

