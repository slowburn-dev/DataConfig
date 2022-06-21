#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/DcEnv.h"
#include "UObject/TextProperty.h"

static FORCEINLINE DcPropertyWriterDetails::FWriteState::ImplStorageType* GetTopStorage(FDcPropertyWriter* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FDcBaseWriteState& AsWriteState(DcPropertyWriterDetails::FWriteState::ImplStorageType* Storage)
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
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateNil>(GetTopStorage(Writer));
}

static FDcWriteStateClass& PushClassRootState(FDcPropertyWriter* Writer, UObject* InClassObject, UClass* InClass)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateClass>(GetTopStorage(Writer), InClassObject, InClass);
}

static FDcWriteStateClass& PushClassPropertyState(FDcPropertyWriter* Writer, void* InDataPtr, FObjectProperty* InObjProperty, FDcClassAccess::EControl InConfigControl)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateClass>(GetTopStorage(Writer), InDataPtr, InObjProperty, InConfigControl);
}

static FDcWriteStateStruct& PushStructPropertyState(FDcPropertyWriter* Writer, void* InStructPtr, UScriptStruct* InStructStruct, const FName& InStructName)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateStruct>(GetTopStorage(Writer), InStructPtr, InStructStruct, InStructName);
}

static FDcWriteStateMap& PushMappingPropertyState(FDcPropertyWriter* Writer, void* InMapPtr, FMapProperty* InMapProperty)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateMap>(GetTopStorage(Writer), InMapPtr, InMapProperty);
}

static FDcWriteStateMap& PushMappingPropertyState(FDcPropertyWriter* Writer, FProperty* InKeyProperty, FProperty* InValueProperty, void* InMap, EMapPropertyFlags InMapFlags)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateMap>(GetTopStorage(Writer), InKeyProperty, InValueProperty, InMap, InMapFlags);
}

static FDcWriteStateArray& PushArrayPropertyState(FDcPropertyWriter* Writer, void* InArrayPtr, FArrayProperty* InArrayProperty)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateArray>(GetTopStorage(Writer), InArrayPtr, InArrayProperty);
}

static FDcWriteStateArray& PushArrayPropertyState(FDcPropertyWriter* Writer, FProperty* InInnerProperty, void* InArray, EArrayPropertyFlags InArrayFlags)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateArray>(GetTopStorage(Writer), InInnerProperty, InArray, InArrayFlags);
}

static FDcWriteStateSet& PushSetPropertyState(FDcPropertyWriter* Writer, void* InSetPtr, FSetProperty* InSetProperty)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateSet>(GetTopStorage(Writer), InSetPtr, InSetProperty);
}

static FDcWriteStateSet& PushSetPropertyState(FDcPropertyWriter* Writer, FProperty* ElementProperty, void* InSet)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateSet>(GetTopStorage(Writer), ElementProperty, InSet);
}

static FDcWriteStateScalar& PushScalarPropertyState(FDcPropertyWriter* Writer, void* InPtr, FProperty* InField)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateScalar>(GetTopStorage(Writer), InPtr, InField);
}

static FDcWriteStateScalar& PushScalarArrayPropertyState(FDcPropertyWriter* Writer, void* InPtr, FProperty* InField)
{
	Writer->States.AddUninitialized();
	return Emplace<FDcWriteStateScalar>(GetTopStorage(Writer), FDcWriteStateScalar::Array, InPtr, InField);
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

	return WriteValue<TProperty, TScalar>(Self, GetTopState(Self), Value);
}

FDcPropertyWriter::FDcPropertyWriter()
{
	Config = FDcPropertyConfig::MakeDefault();
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
		UObject* Obj = (UObject*)(Datum.DataPtr);
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
	else if (Datum.Property.IsA<FArrayProperty>())
	{
		PushArrayPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FArrayProperty>());
	}
	else if (Datum.Property.IsA<FSetProperty>())
	{
		PushSetPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FSetProperty>());
	}
	else if (Datum.Property.IsA<FMapProperty>())
	{
		PushMappingPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FMapProperty>());
	}
	else
	{
		PushScalarPropertyState(this, Datum.DataPtr, Datum.CastField<FProperty>());
	}
}

FDcPropertyWriter::FDcPropertyWriter(EArrayWriter, FProperty* InInnerProperty, void* InArray, EArrayPropertyFlags InArrayFlags)
	: FDcPropertyWriter()
{
	PushArrayPropertyState(this, InInnerProperty, InArray, InArrayFlags);
}

FDcPropertyWriter::FDcPropertyWriter(ESetWriter, FProperty* InElementProperty, void* InSet)
	: FDcPropertyWriter()
{
	PushSetPropertyState(this, InElementProperty, InSet);
}

FDcPropertyWriter::FDcPropertyWriter(FProperty* InKeyProperty, FProperty* InValueProperty, void* InMap, EMapPropertyFlags InMapFlags)
	: FDcPropertyWriter()
{
	PushMappingPropertyState(this, InKeyProperty, InValueProperty, InMap, InMapFlags);
}

FDcResult FDcPropertyWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	return GetTopState(this).PeekWrite(this, Next, bOutOk);
}

FDcResult FDcPropertyWriter::WriteBool(bool Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteString(const FString& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteText(const FText& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteName(const FName& Value)
{
	return GetTopState(this).WriteName(this, Value);
}

FDcResult FDcPropertyWriter::WriteEnum(const FDcEnumData& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).WriteDataEntry(this, FProperty::StaticClass(), Datum));

	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;

	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(Datum.Property, Enum, UnderlyingProperty);
	if (!bIsEnum)
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
			<< TEXT("EnumProperty")  << TEXT("<NumericProperty with Enum>")
			<< Datum.Property.GetFName() << Datum.Property.GetClassName();

	bool bIsUnsigned = DcPropertyUtils::IsUnsignedProperty(UnderlyingProperty);
	FName EnumName = Enum ? Enum->GetFName() : NAME_None;

	if (Value.Flag & FDcEnumData::WriteCheckType
		&& Value.Type != EnumName)
	{
		return DC_FAIL(DcDReadWrite, EnumNameMismatch)
			<< Value.Type <<EnumName 
			<< FormatHighlight();
	}

	if (Value.Flag & FDcEnumData::WriteCheckNameAndValue)
	{
		if (Enum == nullptr)
			return DC_FAIL(DcDReadWrite, EnumMissingEnum)
				<< Datum.Property.GetFName() << Datum.Property.GetClassName();

		if (Value.Name != Enum->GetNameByValue(Value.Signed64))
		{
			return DC_FAIL(DcDReadWrite, EnumNameNotFound)
				<< EnumName << Value.Name
				<< FormatHighlight();
		}

		//	only check validness when there's name, otherwise it might be flags
		if (!Enum->IsValidEnumValue(Value.Signed64))
		{
			return DC_FAIL(DcDReadWrite, EnumValueInvalid)
				<< EnumName << Value.Signed64
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
		UnderlyingProperty->SetIntPropertyValue(Datum.DataPtr, Value.Unsigned64);
	else
		UnderlyingProperty->SetIntPropertyValue(Datum.DataPtr, Value.Signed64);

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteStructRootAccess(FDcStructAccess& Access)
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateStruct* StructState = TopState.As<FDcWriteStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FDcWriteStateStruct::EState::ExpectRoot)
		{
			return StructState->WriteStructRootAccess(this, Access);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(this, FStructProperty::StaticClass(), Datum));

		FStructProperty* StructProperty = Datum.CastFieldChecked<FStructProperty>();
		FDcWriteStateStruct& ChildStruct = PushStructPropertyState(
			this,
			Datum.DataPtr,
			StructProperty->Struct,
			StructProperty->GetFName()
		);
		DC_TRY(ChildStruct.WriteStructRootAccess(this, Access));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteStructEndAccess(FDcStructAccess& Access)
{
	if (FDcWriteStateStruct* StructState = TryGetTopState<FDcWriteStateStruct>(this))
	{
		DC_TRY(StructState->WriteStructEndAccess(this, Access));
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

FDcResult FDcPropertyWriter::PushTopClassPropertyState(const FDcPropertyDatum& Datum)
{
	if (FObjectProperty* ObjProperty = Datum.CastField<FObjectProperty>())
	{
		PushClassPropertyState(this, Datum.DataPtr, ObjProperty, Config.ShouldExpandObject(ObjProperty)
			? FDcClassAccess::EControl::ExpandObject
			: FDcClassAccess::EControl::ReferenceOrNil);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< TEXT("ObjectProperty") << Datum.Property.GetFName() << Datum.Property.GetClassName();
	}
}

FDcResult FDcPropertyWriter::PushTopStructPropertyState(const FDcPropertyDatum& Datum, const FName& StructName)
{
	if (FStructProperty* StructProperty = Datum.CastField<FStructProperty>())
	{
		PushStructPropertyState(
			this,
			Datum.DataPtr,
			StructProperty->Struct,
			StructProperty->GetFName()
		);
		return DcOk();
	}
	else if (UScriptStruct* StructClass = Datum.CastUScriptStruct())
	{
		PushStructPropertyState(this, Datum.DataPtr, StructClass, StructName);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
				<< TEXT("StructProperty") << TEXT("ScriptStruct")
				<< Datum.Property.GetFName() << Datum.Property.GetClassName();
	}
}

bool FDcPropertyWriter::IsWritingScalarArrayItem()
{
	if (FDcWriteStateScalar* ScalarState = GetTopState(this).As<FDcWriteStateScalar>())
		return ScalarState->State == FDcWriteStateScalar::EState::ExpectArrayItem;

	return false;
}

FDcResult FDcPropertyWriter::SetConfig(FDcPropertyConfig InConfig)
{
	Config = MoveTemp(InConfig);
	return Config.Prepare();
}

FDcResult FDcPropertyWriter::WriteClassRootAccess(FDcClassAccess& Access)
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateClass* ClassState = TopState.As<FDcWriteStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FDcWriteStateClass::EState::ExpectRoot)
		{
			return ClassState->WriteClassRootAccess(this, Access);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(this, FObjectProperty::StaticClass(), Datum));

		DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Datum.DataPtr));

		FObjectProperty* ObjProperty = Datum.CastFieldChecked<FObjectProperty>();

		FDcClassAccess::EControl ConfigControl = Access.Control;
		if (ConfigControl == FDcClassAccess::EControl::Default)
		{
			ConfigControl = Config.ShouldExpandObject(ObjProperty)
				? FDcClassAccess::EControl::ExpandObject
				: FDcClassAccess::EControl::ReferenceOrNil;
		}

		PushClassPropertyState(this, Datum.DataPtr, ObjProperty, ConfigControl);
		DC_TRY(GetTopState(this).As<FDcWriteStateClass>()->WriteClassRootAccess(this, Access));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteClassEndAccess(FDcClassAccess& Access)
{
	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		DC_TRY(ClassState->WriteClassEndAccess(this, Access));
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
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateMap* MapState = TopState.As<FDcWriteStateMap>();
		if (MapState != nullptr
			&& MapState->State == FDcWriteStateMap::EState::ExpectRoot)
		{
			return MapState->WriteMapRoot(this);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(this, FMapProperty::StaticClass(), Datum));

		FDcWriteStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FMapProperty>());
		DC_TRY(ChildMap.WriteMapRoot(this));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteMapEnd()
{
	if (FDcWriteStateMap* MapState = TryGetTopState<FDcWriteStateMap>(this))
	{
		DC_TRY(MapState->WriteMapEnd(this));
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
	FDcBaseWriteState& TopState = GetTopState(this);

	{
		if (FDcWriteStateArray* ArrayState = TopState.As<FDcWriteStateArray>())
		{
			if (ArrayState->State == FDcWriteStateArray::EState::ExpectRoot)
				return ArrayState->WriteArrayRoot(this);
		}
		else if (FDcWriteStateScalar* ScalarState = TopState.As<FDcWriteStateScalar>())
		{
			if (ScalarState->State == FDcWriteStateScalar::EState::ExpectArrayRoot)
				return ScalarState->WriteArrayRoot(this);	
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(this, FProperty::StaticClass(), Datum));

		if (Datum.IsA<FArrayProperty>())
		{
			FDcWriteStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FArrayProperty>());
			DC_TRY(ChildArray.WriteArrayRoot(this));
		}
		else if (DcPropertyUtils::IsScalarArray(Datum.Property))
		{
			FDcWriteStateScalar& ChildScalarArray = PushScalarArrayPropertyState(this, Datum.DataPtr, Datum.CastField<FProperty>());
			DC_TRY(ChildScalarArray.WriteArrayRoot(this));
		}
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteArrayEnd()
{
	FDcBaseWriteState& TopState = GetTopState(this);
	if (FDcWriteStateArray* ArrayState = TopState.As<FDcWriteStateArray>())
	{
		DC_TRY(ArrayState->WriteArrayEnd(this));
		PopState<FDcWriteStateArray>(this);
		return DcOk();
	}
	else if (FDcWriteStateScalar* ScalarState = TopState.As<FDcWriteStateScalar>())
	{
		DC_TRY(ScalarState->WriteArrayEnd(this));
		PopState<FDcWriteStateScalar>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateArray::ID << (int)TopState.GetType()
			<< FormatHighlight();
	}
}

FDcResult FDcPropertyWriter::WriteSetRoot()
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateSet* SetState = TopState.As<FDcWriteStateSet>();
		if (SetState != nullptr
			&& SetState->State == FDcWriteStateSet::EState::ExpectRoot)
		{
			return SetState->WriteSetRoot(this);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(this, FSetProperty::StaticClass(), Datum));

		FDcWriteStateSet& ChildSet = PushSetPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FSetProperty>());
		DC_TRY(ChildSet.WriteSetRoot(this));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteSetEnd()
{
	if (FDcWriteStateSet* SetState = TryGetTopState<FDcWriteStateSet>(this))
	{
		DC_TRY(SetState->WriteSetEnd(this));
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
	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		return ClassState->WriteNil(this);
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
	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		return ClassState->WriteObjectReference(this, Value);
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
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).WriteDataEntry(this, FMulticastSparseDelegateProperty::StaticClass(), Datum));

	FMulticastSparseDelegateProperty* SparseProperty = Datum.CastFieldChecked<FMulticastSparseDelegateProperty>();
	SparseProperty->SetMulticastDelegate(Datum.DataPtr, Value);

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value) { return WriteTopStateScalarProperty(this, Value); }
FDcResult FDcPropertyWriter::WriteInterfaceReference(const FScriptInterface& Value) { return WriteTopStateScalarProperty(this, Value); }

FDcResult FDcPropertyWriter::WriteSoftObjectReference(const FSoftObjectPtr& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).WriteDataEntry(this, FSoftObjectProperty::StaticClass(), Datum));

	FSoftObjectProperty* SoftObjectProperty = Datum.CastFieldChecked<FSoftObjectProperty>();
	SoftObjectProperty->SetPropertyValue(Datum.DataPtr, Value);

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteSoftClassReference(const FSoftObjectPtr& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).WriteDataEntry(this, FSoftClassProperty::StaticClass(), Datum));

	FSoftClassProperty* SoftClassProperty = Datum.CastFieldChecked<FSoftClassProperty>();
	SoftClassProperty->SetPropertyValue(Datum.DataPtr, Value);

	return DcOk();
}

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
	FFieldVariant NextProperty;
	DC_TRY(GetTopState(this).PeekWriteProperty(this, &NextProperty));

	FProperty* Prop =  CastField<FProperty>(NextProperty.ToField());
	if (Prop == nullptr)
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::ArrayRoot << EDcDataEntry::StructRoot
			<< DcPropertyUtils::PropertyToDataEntry(NextProperty);

	if (Prop->IsA<FArrayProperty>())
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).WriteDataEntry(this, FArrayProperty::StaticClass(), Datum));
		DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value.DataPtr));

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
	else if (Prop->ArrayDim > 1)
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).WriteDataEntry(this, FProperty::StaticClass(), Datum));
		DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value.DataPtr));

		int FullSize = Prop->ArrayDim * Prop->ElementSize;
		if (Value.Num > FullSize)
			return DC_FAIL(DcDReadWrite, WriteBlobOverrun) << FullSize << Value.Num;

		FMemory::Memcpy(Datum.DataPtr, Value.DataPtr, Value.Num);
		return DcOk();
	}
	else if (Prop->IsA<FStructProperty>())
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).WriteDataEntry(this, FStructProperty::StaticClass(), Datum));
		DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value.DataPtr));

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
	return GetTopState(this).SkipWrite(this);
}

FDcResult FDcPropertyWriter::PeekWriteProperty(FFieldVariant* OutProperty)
{
	return GetTopState(this).PeekWriteProperty(this, OutProperty);
}

FDcResult FDcPropertyWriter::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	return GetTopState(this).WriteDataEntry(this, ExpectedPropertyClass, OutDatum);
}

FDcDiagnosticHighlight FDcPropertyWriter::FormatHighlight()
{
	FDcDiagnosticHighlight OutHighlight(this, ClassId().ToString());
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

	FString Path = Segments.Num() == 0
		? TEXT("<nil>")
		: FString::Join(Segments, TEXT("."));
	OutHighlight.Formatted = FString::Printf(TEXT("Writing property: %s"), *Path);
	return OutHighlight;
}

void FDcPropertyWriter::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Diag << FormatHighlight();
}

FName FDcPropertyWriter::ClassId() { return FName(TEXT("DcPropertyWriter")); }
FName FDcPropertyWriter::GetId() { return ClassId(); }

