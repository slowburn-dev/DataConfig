#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/DcEnv.h"
#include "UObject/TextProperty.h"

static FORCEINLINE FDcPropertyWriter::FPropertyState::ImplStorageType* GetTopStorage(FDcPropertyWriter* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FDcBaseWriteState& AsWriteState(FDcPropertyWriter::FPropertyState::ImplStorageType* Storage)
{
	return *reinterpret_cast<FDcBaseWriteState*>(Storage);
}

static FORCEINLINE FDcBaseWriteState& GetTopState(FDcPropertyWriter* Self)
{
	return AsWriteState(GetTopStorage(Self));
}

template<typename TState>
static TState& GetTopState(FDcPropertyWriter* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FDcPropertyWriter* Self) {
	return GetTopState(Self).As<TState>();
}

static FDcWriteStateNil& PushNilState(FDcPropertyWriter* Writer) {
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateNil>(GetTopStorage(Writer));
}

static FDcWriteStateClass& PushClassRootState(FDcPropertyWriter* Writer, UObject* InClassObject, UClass* InClass)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateClass>(GetTopStorage(Writer), InClassObject, InClass);
}

static FDcWriteStateClass& PushClassPropertyState(FDcPropertyWriter* Writer, void* InDataPtr, FObjectProperty* InObjProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateClass>(GetTopStorage(Writer), InDataPtr, InObjProperty);
}

static FDcWriteStateStruct& PushStructPropertyState(FDcPropertyWriter* Writer, void* InStructPtr, UScriptStruct* InStructStruct, const FName& InStructName)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateStruct>(GetTopStorage(Writer), InStructPtr, InStructStruct, InStructName);
}

static FDcWriteStateMap& PushMappingPropertyState(FDcPropertyWriter* Writer, void* InMapPtr, FMapProperty* InMapProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateMap>(GetTopStorage(Writer), InMapPtr, InMapProperty);
}

static FDcWriteStateArray& PushArrayPropertyState(FDcPropertyWriter* Writer, void* InArrayPtr, FArrayProperty* InArrayProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateArray>(GetTopStorage(Writer), InArrayPtr, InArrayProperty);
}

static FDcWriteStateSet& PushSetPropertyState(FDcPropertyWriter* Writer, void* InSetPtr, FSetProperty* InSetProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateSet>(GetTopStorage(Writer), InSetPtr, InSetProperty);
}

static void PopState(FDcPropertyWriter* Writer)
{
	Writer->States.Pop();
	check(Writer->States.Num() >= 1);
}

template<typename TState>
static void PopState(FDcPropertyWriter* Writer)
{
	check(TState::ID == GetTopState(Writer).GetType());
	PopState(Writer);
}

template<typename TScalar>
FORCEINLINE FDcResult WriteTopStateScalarProperty(FDcPropertyWriter* Self, const TScalar& Value)
{
	using TProperty = typename DcPropertyUtils::TPropertyTypeMap<TScalar>::Type;

	FScopedStackedWriter StackedWriter(Self);

	return WriteValue<TProperty, TScalar>(GetTopState(Self), Value);
}

FDcPropertyWriter::FDcPropertyWriter()
{
	PushNilState(this);
}

FDcPropertyWriter::FDcPropertyWriter(FDcPropertyDatum Datum)
	: FDcPropertyWriter()
{
	if (Datum.IsNone())
	{
		//	pass
	}
	else if (Datum.Property.IsA<UClass>())
	{
		UObject* Obj = reinterpret_cast<UObject*>(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassRootState(this, Obj, Datum.CastUClassChecked());
	}
	else if (Datum.Property.IsA<UScriptStruct>())
	{
		PushStructPropertyState(this,
			Datum.DataPtr,
			Datum.CastUScriptStructChecked(),
			FName(TEXT("$root"))
		);
	}
	else
	{
		checkNoEntry();
	}
}

FDcResult FDcPropertyWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	FScopedStackedWriter StackedWriter(this);
	return GetTopState(this).PeekWrite(Next, bOutOk);
}

FDcResult FDcPropertyWriter::WriteBool(bool Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteString(const FString& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteText(const FText& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteName(const FName& Value)
{
	FScopedStackedWriter StackedWriter(this);

	return GetTopState(this).WriteName(Value);
}

FDcResult FDcPropertyWriter::WriteEnum(const FDcEnumData& Value)
{
	FScopedStackedWriter StackedWriter(this);

	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).WriteDataEntry(FEnumProperty::StaticClass(), Datum));

	FEnumProperty* EnumProperty = Datum.CastFieldChecked<FEnumProperty>();
	FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	bool bIsUnsigned = DcPropertyUtils::IsUnsignedProperty(UnderlyingProperty);

	UEnum* Enum = EnumProperty->GetEnum();

	if (Value.Flag & FDcEnumData::WriteCheckType
		&& Value.Type != Enum->GetFName())
	{
		return DC_FAIL(DcDReadWrite, EnumNameMismatch)
			<< Value.Type << Enum->GetName()
			<< FormatHighlight();
	}

	if (Value.Flag & FDcEnumData::WriteCheckNameAndValue
		&& Value.Name != Enum->GetNameByValue(Value.Signed64))
	{
		return DC_FAIL(DcDReadWrite, EnumNameNotFound)
			<< Enum->GetName() << Value.Name
			<< FormatHighlight();

		//	only check validness when there's name, otherwise it might be flags
		if (!Enum->IsValidEnumValue(Value.Signed64))
		{
			return DC_FAIL(DcDReadWrite, EnumValueInvalid)
				<< Enum->GetName() << Value.Signed64
				<< FormatHighlight();
		}
	}

	if (Value.Flag & FDcEnumData::WriteCheckSign
		&& bIsUnsigned != Value.bIsUnsigned)
	{
		return DC_FAIL(DcDReadWrite, EnumSignMismatch)
			<< bIsUnsigned << Value.bIsUnsigned
			<< FormatHighlight();
	}

	if (bIsUnsigned)
		EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Datum.DataPtr, Value.Unsigned64);
	else
		EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Datum.DataPtr, Value.Signed64);

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteStructRoot(const FDcStructStat& Struct)
{
	FScopedStackedWriter StackedWriter(this);

	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateStruct* StructState = TopState.As<FDcWriteStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FDcWriteStateStruct::EState::ExpectRoot)
		{
			return StructState->WriteStructRoot(Struct);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(FStructProperty::StaticClass(), Datum));

		FStructProperty* StructProperty = Datum.CastFieldChecked<FStructProperty>();
		FDcWriteStateStruct& ChildStruct = PushStructPropertyState(
			this,
			Datum.DataPtr,
			StructProperty->Struct,
			StructProperty->GetFName()
		);
		DC_TRY(ChildStruct.WriteStructRoot(Struct));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteStructEnd(const FDcStructStat& Struct)
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateStruct* StructState = TryGetTopState<FDcWriteStateStruct>(this))
	{
		DC_TRY(StructState->WriteStructEnd(Struct));
		PopState<FDcWriteStateStruct>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateStruct::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();
	}
}

void FDcPropertyWriter::PushTopClassPropertyState(const FDcPropertyDatum& Datum)
{
	FObjectProperty* ObjProperty = Datum.CastFieldChecked<FObjectProperty>();
	PushClassPropertyState(this, Datum.DataPtr, ObjProperty);
}


void FDcPropertyWriter::PushTopStructPropertyState(const FDcPropertyDatum& Datum, const FName& StructName)
{
	UScriptStruct* StructClass = Datum.CastUScriptStructChecked();
	PushStructPropertyState(this, Datum.DataPtr, StructClass, StructName);
}

FDcResult FDcPropertyWriter::WriteClassRoot(const FDcClassStat& Class)
{
	FScopedStackedWriter StackedWriter(this);

	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateClass* ClassState = TopState.As<FDcWriteStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FDcWriteStateClass::EState::ExpectRoot)
		{
			return ClassState->WriteClassRoot(Class);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(FObjectProperty::StaticClass(), Datum));

		PushTopClassPropertyState(Datum);
		DC_TRY(GetTopState(this).As<FDcWriteStateClass>()->WriteClassRoot(Class));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteClassEnd(const FDcClassStat& Class)
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		DC_TRY(ClassState->WriteClassEnd(Class));
		PopState<FDcWriteStateClass>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateClass::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();
	}
}

FDcResult FDcPropertyWriter::WriteMapRoot()
{
	FScopedStackedWriter StackedWriter(this);

	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateMap* MapState = TopState.As<FDcWriteStateMap>();
		if (MapState != nullptr
			&& MapState->State == FDcWriteStateMap::EState::ExpectRoot)
		{
			checkNoEntry();	// TODO remove if this isn't really used
			return MapState->WriteMapRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(FMapProperty::StaticClass(), Datum));

		FDcWriteStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FMapProperty>());
		DC_TRY(ChildMap.WriteMapRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteMapEnd()
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateMap* MapState = TryGetTopState<FDcWriteStateMap>(this))
	{
		DC_TRY(MapState->WriteMapEnd());
		PopState<FDcWriteStateMap>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateMap::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();

	}
}

FDcResult FDcPropertyWriter::WriteArrayRoot()
{
	FScopedStackedWriter StackedWriter(this);

	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateArray* ArrayState = TopState.As<FDcWriteStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FDcWriteStateArray::EState::ExpectRoot)
		{
			checkNoEntry();	// TODO remove if this isn't really used
			return ArrayState->WriteArrayRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(FArrayProperty::StaticClass(), Datum));

		FDcWriteStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FArrayProperty>());
		DC_TRY(ChildArray.WriteArrayRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteArrayEnd()
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateArray* ArrayState = TryGetTopState<FDcWriteStateArray>(this))
	{
		DC_TRY(ArrayState->WriteArrayEnd());
		PopState<FDcWriteStateArray>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateArray::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();
	}
}

FDcResult FDcPropertyWriter::WriteSetRoot()
{
	FScopedStackedWriter StackedWriter(this);

	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateSet* SetState = TopState.As<FDcWriteStateSet>();
		if (SetState != nullptr
			&& SetState->State == FDcWriteStateSet::EState::ExpectRoot)
		{
			checkNoEntry();	// TODO remove if this isn't really used
			return SetState->WriteSetRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(FSetProperty::StaticClass(), Datum));

		FDcWriteStateSet& ChildSet = PushSetPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FSetProperty>());
		DC_TRY(ChildSet.WriteSetRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteSetEnd()
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateSet* SetState = TryGetTopState<FDcWriteStateSet>(this))
	{
		DC_TRY(SetState->WriteSetEnd());
		PopState<FDcWriteStateSet>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateSet::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();
	}
}

FDcResult FDcPropertyWriter::WriteNil()
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		return ClassState->WriteNil();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateClass::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();
	}
}

FDcResult FDcPropertyWriter::WriteObjectReference(const UObject* Value)
{
	FScopedStackedWriter StackedWriter(this);

	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		return ClassState->WriteObjectReference(Value);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateClass::ID << (int)GetTopState(this).GetType()
			<< FormatHighlight();
	}
}

FDcResult FDcPropertyWriter::WriteClassReference(const UClass* Value)
{
	//	ignore constness here for simpler code
	return WriteTopStateScalarProperty<UClass*>(this, (UClass*)Value);
}

FDcResult FDcPropertyWriter::WriteFieldPath(const FFieldPath& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteDelegate(const FScriptDelegate& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value)
{
	FScopedStackedWriter StackedWriter(this);

	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).WriteDataEntry(FMulticastSparseDelegateProperty::StaticClass(), Datum));

	FMulticastSparseDelegateProperty* SparseProperty = Datum.CastFieldChecked<FMulticastSparseDelegateProperty>();
	SparseProperty->SetMulticastDelegate(Datum.DataPtr, Value);

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteSoftObjectReference(const FSoftObjectPath& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteSoftClassReference(const FSoftClassPath& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteInterfaceReference(const FScriptInterface& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteInt8(const int8& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteInt16(const int16& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteInt32(const int32& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteInt64(const int64& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteUInt8(const uint8& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteUInt16(const uint16& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteUInt32(const uint32& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteUInt64(const uint64& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteFloat(const float& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteDouble(const double& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteBlob(const FDcBlobViewData& Value)
{
	FScopedStackedWriter StackedWriter(this);

	FFieldVariant NextProperty;
	DC_TRY(GetTopState(this).PeekWriteProperty(&NextProperty));

	if (NextProperty.IsA<FArrayProperty>())
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).WriteDataEntry(FArrayProperty::StaticClass(), Datum));

		FArrayProperty* ArrayProperty = Datum.CastFieldChecked<FArrayProperty>();
		FScriptArrayHelper ScriptArray(ArrayProperty, Datum.DataPtr);

		int32 ElementSize = ArrayProperty->Inner->ElementSize;
		int32 NewCount = Value.Num / ElementSize;
		if (Value.Num % ElementSize != 0)
			NewCount += 1;
		ScriptArray.EmptyAndAddUninitializedValues(NewCount);

		FMemory::Memcpy(ScriptArray.GetRawPtr(), Value.DataPtr, Value.Num);
		return DcOk();
	}
	else if (NextProperty.IsA<FStructProperty>())
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).WriteDataEntry(FStructProperty::StaticClass(), Datum));

		FStructProperty* StructProperty = Datum.CastFieldChecked<FStructProperty>();

		if (Value.Num > StructProperty->ElementSize)
			return DC_FAIL(DcDReadWrite, WriteBlobOverrun) << StructProperty->ElementSize << Value.Num;

		StructProperty->CopySingleValue(Datum.DataPtr, Value.DataPtr);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::ArrayRoot << DcPropertyUtils::PropertyToDataEntry(NextProperty);
	}
}

FDcResult FDcPropertyWriter::SkipWrite()
{
	FScopedStackedWriter StackedWriter(this);

	return GetTopState(this).SkipWrite();
}

FDcResult FDcPropertyWriter::PeekWriteProperty(FFieldVariant* OutProperty)
{
	FScopedStackedWriter StackedWriter(this);

	return GetTopState(this).PeekWriteProperty(OutProperty);
}

FDcResult FDcPropertyWriter::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	FScopedStackedWriter StackedWriter(this);

	return GetTopState(this).WriteDataEntry(ExpectedPropertyClass, OutDatum);
}

FDcDiagnosticHighlight FDcPropertyWriter::FormatHighlight()
{
	FDcDiagnosticHighlight OutHighlight(this, TEXT("PropertyWriter"));
	TArray<FString> Segments;

	bool bLastIsContainer = false;
	int Num = States.Num();
	for (int Ix = 1; Ix < Num; Ix++)
	{
		FDcBaseWriteState& WriteState = AsWriteState(&States[Ix].ImplStorage);
		DcPropertyHighlight::EFormatSeg Seg;
		if (bLastIsContainer)
			Seg = DcPropertyHighlight::EFormatSeg::ParentIsContainer;
		else if (Ix == Num - 1)
			Seg = DcPropertyHighlight::EFormatSeg::Last;
		else
			Seg = DcPropertyHighlight::EFormatSeg::Normal;

		WriteState.FormatHighlightSegment(Segments, Seg);

		EDcPropertyWriteType StateType = WriteState.GetType();
		bLastIsContainer = StateType == EDcPropertyWriteType::ArrayProperty
			|| StateType == EDcPropertyWriteType::SetProperty
			|| StateType == EDcPropertyWriteType::MapProperty;
	}

	FString Path = FString::Join(Segments, TEXT("."));
	OutHighlight.Formatted = FString::Printf(TEXT("Writing property: %s"), *Path);
	return OutHighlight;
}

void FDcPropertyWriter::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Diag << FormatHighlight();
}

