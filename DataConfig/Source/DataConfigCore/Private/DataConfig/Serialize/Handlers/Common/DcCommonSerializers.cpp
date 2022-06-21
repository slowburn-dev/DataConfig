#include "DataConfig/Serialize/Handlers/Common/DcCommonSerializers.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"

namespace DcCommonHandlers {

void AddNumericPipeDirectHandlers(FDcSerializer& Serializer)
{
	Serializer.AddDirectHandler(FInt8Property::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeInt8Serialize));
	Serializer.AddDirectHandler(FInt16Property::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeInt16Serialize));
	Serializer.AddDirectHandler(FIntProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeInt32Serialize));
	Serializer.AddDirectHandler(FInt64Property::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeInt64Serialize));

	Serializer.AddDirectHandler(FByteProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeUInt8Serialize));
	Serializer.AddDirectHandler(FUInt16Property::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeUInt16Serialize));
	Serializer.AddDirectHandler(FUInt32Property::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeUInt32Serialize));
	Serializer.AddDirectHandler(FUInt64Property::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeUInt64Serialize));

	Serializer.AddDirectHandler(FFloatProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeFloatSerialize));
	Serializer.AddDirectHandler(FDoubleProperty::StaticClass(), FDcSerializeDelegate::CreateStatic(HandlerPipeDoubleSerialize));
}

EDcSerializePredicateResult PredicateIsScalarArrayProperty(FDcSerializeContext& Ctx)
{
	FProperty* Prop = CastField<FProperty>(Ctx.TopProperty().ToField());
	return Prop && Prop->ArrayDim > 1 && !Ctx.Reader->IsReadingScalarArrayItem()
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

FDcResult HandlerPipeScalarSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeScalar(Ctx);
}

FDcResult HandlerPipeBoolSerialize(FDcSerializeContext& Ctx) { return DcPipe_Bool(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeNameSerialize(FDcSerializeContext& Ctx) { return DcPipe_Name(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeStringSerialize(FDcSerializeContext& Ctx) { return DcPipe_String(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeTextSerialize(FDcSerializeContext& Ctx) { return DcPipe_Text(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeFloatSerialize(FDcSerializeContext& Ctx) { return DcPipe_Float(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeDoubleSerialize(FDcSerializeContext& Ctx) { return DcPipe_Double(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeInt8Serialize(FDcSerializeContext& Ctx) { return DcPipe_Int8(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeInt16Serialize(FDcSerializeContext& Ctx) { return DcPipe_Int16(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeInt32Serialize(FDcSerializeContext& Ctx) { return DcPipe_Int32(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeInt64Serialize(FDcSerializeContext& Ctx) { return DcPipe_Int64(Ctx.Reader, Ctx.Writer); }

FDcResult HandlerPipeUInt8Serialize(FDcSerializeContext& Ctx) { return DcPipe_UInt8(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeUInt16Serialize(FDcSerializeContext& Ctx) { return DcPipe_UInt16(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeUInt32Serialize(FDcSerializeContext& Ctx) { return DcPipe_UInt32(Ctx.Reader, Ctx.Writer); }
FDcResult HandlerPipeUInt64Serialize(FDcSerializeContext& Ctx) { return DcPipe_UInt64(Ctx.Reader, Ctx.Writer); }

EDcSerializePredicateResult PredicateIsEnumProperty(FDcSerializeContext& Ctx)
{
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(Ctx.TopProperty(), Enum, UnderlyingProperty);
	
	return bIsEnum
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

FDcResult HandlerEnumToStringSerialize(FDcSerializeContext& Ctx)
{
	FFieldVariant& TopProperty = Ctx.TopProperty();
	UEnum* Enum;
	FNumericProperty* UnderlyingProperty;
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(TopProperty, Enum, UnderlyingProperty);

	if (!bIsEnum)
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
			<< TEXT("EnumProperty")  << TEXT("<NumericProperty with Enum>")
			<< TopProperty.GetFName() << TopProperty.GetClassName();

	FDcEnumData Value;
	DC_TRY(Ctx.Reader->ReadEnum(&Value));

	if (Enum == nullptr)
	{
		//	fallback to write numeric
		return Ctx.Writer->WriteUInt64(Value.Unsigned64);
	}

	bool bIsBitFlags;
#if WITH_METADATA
	bIsBitFlags = Enum->HasMetaData(TEXT("Bitflags"));
#else
	//	Program target is missing `UEnum::HasMetaData`
	bIsBitFlags = ((UField*)Enum)->HasMetaData(TEXT("Bitflags"));
#endif

	if (!bIsBitFlags)
	{
		int ValueIndex = Enum->GetIndexByValue(Value.Signed64);
		if (ValueIndex == INDEX_NONE)
			return DC_FAIL(DcDReadWrite, EnumValueInvalid)
				<< Enum->GetName() << Value.Signed64;

		DC_TRY(Ctx.Writer->WriteString(Enum->GetNameStringByValue(Value.Signed64)));
	}
	else
	{
		DC_TRY(Ctx.Writer->WriteArrayRoot());

		int Len = Enum->NumEnums();
		uint64 Data = Value.Unsigned64;
		for (int Ix = 0; Ix < Len; Ix++)
		{
			int64 Cur = Enum->GetValueByIndex(Ix);
			int CurPopCount = FPlatformMath::CountBits(Cur);
			if (CurPopCount == 0)
				continue;

			if (FPlatformMath::CountBits(Data & Cur) == CurPopCount)
			{
				DC_TRY(Ctx.Writer->WriteString(Enum->GetNameStringByIndex(Ix)));
				Data ^= Cur;
			}
		}

		if (Data != 0)
			return DC_FAIL(DcDSerDe, EnumBitFlagsNotFullyMasked)
				<< Enum->GetName();

		DC_TRY(Ctx.Writer->WriteArrayEnd());
	}

	return DcOk();
}

FDcResult HandlerFieldPathToStringDeserialize(FDcSerializeContext& Ctx)
{
	FFieldPath Value;
	DC_TRY(Ctx.Reader->ReadFieldPath(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	return DcOk();
}

FDcResult HandlerStructToMapSerialize(FDcSerializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadStructRoot());
	DC_TRY(Ctx.Writer->WriteMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));

		if (CurPeek == EDcDataEntry::StructEnd)
		{
			break;
		}
		else if (CurPeek == EDcDataEntry::Name)
		{
			FName Value;
			DC_TRY(Ctx.Reader->ReadName(&Value));
			DC_TRY(Ctx.Writer->WriteName(Value));
		}
		else
		{
			return DC_FAIL(DcDSerDe, DataEntryMismatch)
				<< EDcDataEntry::Name << CurPeek;
		}

		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadStructEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

FDcResult HandlerClassToMapSerialize(FDcSerializeContext& Ctx)
{
	FDcClassAccess Access{FDcClassAccess::EControl::ExpandObject};
	DC_TRY(Ctx.Reader->ReadClassRootAccess(Access));
	DC_TRY(Ctx.Writer->WriteMapRoot());


	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ClassEnd)
		{
			break;
		}
		else if (CurPeek == EDcDataEntry::Name)
		{
			FName Value;
			DC_TRY(Ctx.Reader->ReadName(&Value));
			DC_TRY(Ctx.Writer->WriteName(Value));
		}
		else
		{
			return DC_FAIL(DcDSerDe, DataEntryMismatch)
				<< EDcDataEntry::Name << CurPeek;
		}

		DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOk();
}

FDcResult HandlerArraySerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		EDcDataEntry::ArrayEnd,
		&DcSerializeUtils::RecursiveSerialize,
		&FDcPropertyReader::ReadArrayRoot,
		&FDcPropertyReader::ReadArrayEnd,
		&FDcWriter::WriteArrayRoot,
		&FDcWriter::WriteArrayEnd
	>(Ctx);
}

FDcResult HandlerSetToArraySerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		EDcDataEntry::SetEnd,
		&DcSerializeUtils::RecursiveSerialize,
		&FDcPropertyReader::ReadSetRoot,
		&FDcPropertyReader::ReadSetEnd,
		&FDcWriter::WriteArrayRoot,
		&FDcWriter::WriteArrayEnd
	>(Ctx);
}

FDcResult HandlerStringKeyMapOrArrayOfKeyValueSerialize(FDcSerializeContext& Ctx)
{
	FFieldVariant Prop;
	DC_TRY(Ctx.Reader->PeekReadProperty(&Prop));

	FMapProperty* MapProp = CastField<FMapProperty>(Prop.ToField());
	if (MapProp == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("MapProperty") << Prop.GetFName() << Prop.GetClassName();

	FProperty* KeyProp = MapProp->KeyProp;
	if (KeyProp->IsA<FStrProperty>()
		|| KeyProp->IsA<FNameProperty>()
		|| KeyProp->IsA<FTextProperty>())
	{
		return DcHandlerPipeMap<
			FDcSerializeContext,
			FDcPropertyReader,
			FDcWriter,
			&DcSerializeUtils::RecursiveSerialize
		>(Ctx);
	}
	else
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		DC_TRY(Ctx.Writer->WriteArrayRoot());

		EDcDataEntry Cur;
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&Cur));
			if (Cur == EDcDataEntry::MapEnd)
				break;

			DC_TRY(Ctx.Writer->WriteMapRoot());

			DC_TRY(Ctx.Writer->WriteString(TEXT("$key")));
			DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));

			DC_TRY(Ctx.Writer->WriteString(TEXT("$value")));
			DC_TRY(DcSerializeUtils::RecursiveSerialize(Ctx));

			DC_TRY(Ctx.Writer->WriteMapEnd());
		}

		DC_TRY(Ctx.Reader->ReadMapEnd());
		DC_TRY(Ctx.Writer->WriteArrayEnd());

		return DcOk();
	}
}


} // namespace DcCommonHandlers

