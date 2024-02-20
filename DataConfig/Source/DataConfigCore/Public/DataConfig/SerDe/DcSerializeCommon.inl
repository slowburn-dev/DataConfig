#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"

FORCEINLINE_DEBUGGABLE FDcResult TryWriteObjectReference(FDcSerializeContext& Ctx, FObjectPropertyBase*, UObject* Value)
{
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value));
	if (Value == nullptr)
		DC_TRY(Ctx.Writer->WriteNone());
	else
		DC_TRY(Ctx.Writer->WriteString(DcSerDeUtils::FormatObjectName(Value)));

	return DcOk();
}

FORCEINLINE_DEBUGGABLE FDcResult TryWriteTypeStr(FDcSerializeContext& Ctx, FObjectPropertyBase* ObjectProperty, UObject* Value)
{
	check(Value != nullptr);
	return Ctx.Writer->WriteName(Value->GetClass()->GetFName());
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

		if (Next == EDcDataEntry::None)
			DC_TRY(Ctx.Reader->ReadNone());
		else if (Next == EDcDataEntry::ObjectReference)
			DC_TRY(Ctx.Reader->ReadObjectReference(&Value));
		else
			return DC_FAIL(DcDSerDe, DataEntryMismatch2)
				<< EDcDataEntry::None << EDcDataEntry::ObjectReference << Next;

		DC_TRY(Ctx.Reader->ReadClassEndAccess(Access));
	}

	Access.Control = FDcClassAccess::EControl::ReferenceOrNone;
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
	DC_TRY(FuncObjectWriter(Ctx, SoftObjectProperty, Value.LoadSynchronous()));

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
	DC_TRY(FuncObjectWriter(Ctx, SoftClassProperty, Value.LoadSynchronous()));

	return DcOk();
}

using FDcTypeWriterSignature = FDcResult(*)(FDcSerializeContext&, FObjectPropertyBase*, UObject*);

template<FDcTypeWriterSignature FuncTypeWriter>
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
	if (CurPeek == EDcDataEntry::None)
	{
		check(SubObjectPtr == nullptr);
		DC_TRY(Ctx.Reader->ReadNone());
		DC_TRY(Ctx.Writer->WriteNone());
	}
	else
	{
		check(SubObjectPtr->IsValidLowLevel());
		DC_TRY(Ctx.Writer->WriteMapRoot());
		DC_TRY(Ctx.Writer->WriteString(TEXT("$type")));
		DC_TRY(FuncTypeWriter(Ctx, ObjectProperty, SubObjectPtr));

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