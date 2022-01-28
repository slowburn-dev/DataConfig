#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"

FORCEINLINE_DEBUGGABLE FDcResult TryWriteObjectReference(FDcSerializeContext& Ctx, FObjectPropertyBase* ObjectProperty, UObject* Value)
{
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value));
	if (Value == nullptr)
		DC_TRY(Ctx.Writer->WriteNil());
	else
		DC_TRY(Ctx.Writer->WriteString(DcSerDeUtils::FormatObjectName(Value)));

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult DcSerializeSoftObjectToString(FDcSerializeContext& Ctx)
{
	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftObjectReference(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult DcSerializeLazyObjectToString(FDcSerializeContext& Ctx)
{
	FLazyObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadLazyObjectReference(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.GetUniqueID().ToString()));
	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult DcSerializeSoftClassToString(FDcSerializeContext& Ctx)
{
	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftClassReference(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	return DcOk();
}

using FDcObjectWriterSignature = FDcResult(*)(FDcSerializeContext&, FObjectPropertyBase*, UObject*);

template<FDcObjectWriterSignature FuncObjectWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcSerializeObjectReference(FDcSerializeContext& Ctx)
{
	FObjectProperty* ObjectProperty = DcPropertyUtils::CastFieldVariant<FObjectProperty>(Ctx.TopProperty());
	if (ObjectProperty == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	if (Ctx.Reader->Config.ShouldExpandObject(ObjectProperty))
		return DC_FAIL(DcDSerDe, SerializerUnexpectedExpandObject)
			<< Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	UObject* Value = nullptr;
	FDcClassAccess Access;

	{
		DC_TRY(Ctx.Reader->ReadClassRootAccess(Access));

		EDcDataEntry Next;
		DC_TRY(Ctx.Reader->PeekRead(&Next));

		if (Next == EDcDataEntry::Nil)
			DC_TRY(Ctx.Reader->ReadNil());
		else if (Next == EDcDataEntry::ObjectReference)
			DC_TRY(Ctx.Reader->ReadObjectReference(&Value));
		else
			return DC_FAIL(DcDSerDe, DataEntryMismatch2)
				<< EDcDataEntry::Nil << EDcDataEntry::ObjectReference << Next;

		DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));
	}

	Access.Control = FDcClassAccess::EControl::ReferenceOrNil;
	DC_TRY(FuncObjectWriter(Ctx, ObjectProperty, Value));

	return DcOk();
}

template<FDcObjectWriterSignature FuncObjectWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcSerializeSoftObjectReference(FDcSerializeContext& Ctx)
{
	FSoftObjectProperty* SoftObjectProperty = DcPropertyUtils::CastFieldVariant<FSoftObjectProperty>(Ctx.TopProperty());
	if (SoftObjectProperty == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("SoftObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftObjectReference(&Value));
	DC_TRY(FuncObjectWriter(Ctx, SoftObjectProperty, Value.Get()));

	return DcOk();
}

template<FDcObjectWriterSignature FuncObjectWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcSerializeWeakObjectReference(FDcSerializeContext& Ctx)
{
	FWeakObjectProperty* WeakObjectProperty = DcPropertyUtils::CastFieldVariant<FWeakObjectProperty>(Ctx.TopProperty());
	if (WeakObjectProperty == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("WeakObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	FWeakObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadWeakObjectReference(&Value));
	DC_TRY(FuncObjectWriter(Ctx, WeakObjectProperty, Value.Get()));

	return DcOk();
}

template<FDcObjectWriterSignature FuncObjectWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcSerializeLazyObjectReference(FDcSerializeContext& Ctx)
{
	FLazyObjectProperty* LazyObjectProperty = DcPropertyUtils::CastFieldVariant<FLazyObjectProperty>(Ctx.TopProperty());
	if (LazyObjectProperty == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("LazyObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	FLazyObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadLazyObjectReference(&Value));
	DC_TRY(FuncObjectWriter(Ctx, LazyObjectProperty, Value.Get()));

	return DcOk();
}

template<FDcObjectWriterSignature FuncObjectWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcSerializeClassReference(FDcSerializeContext& Ctx)
{
	FClassProperty* ClassProperty = DcPropertyUtils::CastFieldVariant<FClassProperty>(Ctx.TopProperty());
	if (ClassProperty == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ClassProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	UClass* Value;
	DC_TRY(Ctx.Reader->ReadClassReference(&Value));
	DC_TRY(FuncObjectWriter(Ctx, ClassProperty, Value));

	return DcOk();
}

template<FDcObjectWriterSignature FuncObjectWriter>
FORCEINLINE_DEBUGGABLE FDcResult DcSerializeSoftClassReference(FDcSerializeContext& Ctx)
{
	FSoftClassProperty* SoftClassProperty = DcPropertyUtils::CastFieldVariant<FSoftClassProperty>(Ctx.TopProperty());
	if (SoftClassProperty == nullptr)
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("SoftClassProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	FSoftObjectPtr Value;
	DC_TRY(Ctx.Reader->ReadSoftClassReference(&Value));
	DC_TRY(FuncObjectWriter(Ctx, SoftClassProperty, Value.Get()));

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult DcSerializeInstancedSubObject(FDcSerializeContext& Ctx)
{
	FObjectProperty* ObjectProperty = DcPropertyUtils::CastFieldVariant<FObjectProperty>(Ctx.TopProperty());
	if (ObjectProperty == nullptr)
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();
	}

	if (!DcPropertyUtils::IsSubObjectProperty(ObjectProperty))
		return DC_FAIL(DcDSerDe, ObjectPropertyNotInline)
			<< ObjectProperty->GetFName() << ObjectProperty->GetClass()->GetFName();

	if (!Ctx.Reader->Config.ShouldExpandObject(ObjectProperty))
		return DC_FAIL(DcDSerDe, SerializerUnexpectedReadReference)
			<< Ctx.TopProperty().GetFName() << Ctx.TopProperty().GetClassName();

	//	peek data ptr early so it points to sub object itself
	UObject* SubObjectPtr;
	{
		void* DataPtr;
		DC_TRY(Ctx.Reader->PeekReadDataPtr(&DataPtr));
		SubObjectPtr = *((UObject**)(DataPtr));
	}

	FDcClassAccess Access;
	DC_TRY(Ctx.Reader->ReadClassRootAccess(Access));

	EDcDataEntry CurPeek;
	DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
	if (CurPeek == EDcDataEntry::Nil)
	{
		check(SubObjectPtr == nullptr);
		DC_TRY(Ctx.Reader->ReadNil());
		DC_TRY(Ctx.Writer->WriteNil());
	}
	else
	{
		check(SubObjectPtr->IsValidLowLevel());
		DC_TRY(Ctx.Writer->WriteMapRoot());
		DC_TRY(Ctx.Writer->WriteString(TEXT("$type")));
		DC_TRY(Ctx.Writer->WriteName(SubObjectPtr->GetClass()->GetFName()));

		while (true)
		{
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

			DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		}

		DC_TRY(Ctx.Writer->WriteMapEnd());
	}

	DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult DcSerializeEnum(FDcSerializeContext& Ctx)
{
	UEnum* Enum;
	FNumericProperty* UnderlyingProperty;

	DC_TRY(DcPropertyUtils::GetEnumProperty(Ctx.TopProperty(), Enum, UnderlyingProperty));

	bool bIsBitFlags;
#if WITH_METADATA
	bIsBitFlags = Enum->HasMetaData(TEXT("Bitflags"));
#else
	//	Program target is missing `UEnum::HasMetaData`
	bIsBitFlags = ((UField*)Enum)->HasMetaData(TEXT("Bitflags"));
#endif

	FDcEnumData Value;
	DC_TRY(Ctx.Reader->ReadEnum(&Value));

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

FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerSerializeStructToMap(FDcSerializeContext& Ctx)
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

FORCEINLINE_DEBUGGABLE static FDcResult DcHandlerSerializeClassToMap(FDcSerializeContext& Ctx)
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

FORCEINLINE_DEBUGGABLE static FDcResult DcSerializeFieldPathToString(FDcSerializeContext& Ctx)
{
	FFieldPath Value;
	DC_TRY(Ctx.Reader->ReadFieldPath(&Value));
	DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
	return DcOk();
}
