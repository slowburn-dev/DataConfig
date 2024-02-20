#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/DcEnv.h"
#include "CoreMinimal.h"
#include "UObject/TextProperty.h"

#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
#include "UObject/PropertyOptional.h"
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

static FORCEINLINE DcPropertyReaderDetails::FReadState::ImplStorageType* GetTopStorage(FDcPropertyReader* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FDcBaseReadState& AsReadState(DcPropertyReaderDetails::FReadState::ImplStorageType* Storage)
{
	return *reinterpret_cast<FDcBaseReadState*>(Storage);
}

static FORCEINLINE FDcBaseReadState& GetTopState(FDcPropertyReader* Self)
{
	return AsReadState(GetTopStorage(Self));
}

template<typename TState>
static TState& GetTopState(FDcPropertyReader* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FDcPropertyReader* Self) {
	return GetTopState(Self).As<TState>();
}

FDcReadStateNone& PushNoneState(FDcPropertyReader* Reader)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateNone>(GetTopStorage(Reader));
}

FDcReadStateClass& PushClassPropertyState(FDcPropertyReader* Reader, UObject* InClassObject, UClass* InClass, FDcReadStateClass::EType InType, const FName& InObjectName)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateClass>(GetTopStorage(Reader), InClassObject, InClass, InType, InObjectName);
}

FDcReadStateStruct& PushStructPropertyState(FDcPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass, const FName& InStructName)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass, InStructName);
}

FDcReadStateMap& PushMappingPropertyState(FDcPropertyReader* Reader, void* InMapPtr, FMapProperty* InMapProperty)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

FDcReadStateMap& PushMappingPropertyState(FDcPropertyReader* Reader, FProperty* InKeyProperty, FProperty* InValueProperty, void* InMap, EMapPropertyFlags InMapFlags)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateMap>(GetTopStorage(Reader), InKeyProperty, InValueProperty, InMap, InMapFlags);
}

FDcReadStateArray& PushArrayPropertyState(FDcPropertyReader* Reader, void* InArrayPtr, FArrayProperty* InArrayProperty)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateArray>(GetTopStorage(Reader), InArrayPtr, InArrayProperty);
}

FDcReadStateArray& PushArrayPropertyState(FDcPropertyReader* Reader, FProperty* InInnerProperty, void *InArray, EArrayPropertyFlags InArrayFlags)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateArray>(GetTopStorage(Reader), InInnerProperty, InArray, InArrayFlags);
}

FDcReadStateSet& PushSetPropertyState(FDcPropertyReader* Reader, void* InSetPtr, FSetProperty* InSetProperty)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateSet>(GetTopStorage(Reader), InSetPtr, InSetProperty);
}

FDcReadStateSet& PushSetPropertyState(FDcPropertyReader* Reader, FProperty* ElementProperty, void* InSet)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateSet>(GetTopStorage(Reader), ElementProperty, InSet);
}

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
FDcReadStateOptional& PushOptionalPropertyState(FDcPropertyReader* Reader, void* InOptionalPtr, FOptionalProperty* InOptionalProperty)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateOptional>(GetTopStorage(Reader), InOptionalPtr, InOptionalProperty);
}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

FDcReadStateScalar& PushScalarPropertyState(FDcPropertyReader* Reader, void* InPtr, FProperty* InField)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateScalar>(GetTopStorage(Reader), InPtr, InField);
}

FDcReadStateScalar& PushScalarArrayPropertyState(FDcPropertyReader* Reader, void* InPtr, FProperty* InField)
{
	Reader->States.AddUninitialized();
	return Emplace<FDcReadStateScalar>(GetTopStorage(Reader), FDcReadStateScalar::Array, InPtr, InField);;
}

void PopState(FDcPropertyReader* Reader)
{
	Reader->States.Pop();
	check(Reader->States.Num() >= 1);
}


template<typename TState>
static void PopState(FDcPropertyReader* Reader)
{
	check(TState::ID == GetTopState(Reader).GetType());
	PopState(Reader);
}

//	for read datum -> scalar
template<typename TProperty, typename TScalar>
void ReadPropertyValueConversion(FField* Property, void const* Ptr, TScalar* OutPtr)
{
	*OutPtr = (const TScalar&)(CastFieldChecked<TProperty>(Property)->GetPropertyValue(Ptr));
}

template<>
void ReadPropertyValueConversion<FBoolProperty, bool>(FField* Property, void const* Ptr, bool* OutPtr)
{
	*OutPtr = CastFieldChecked<FBoolProperty>(Property)->GetPropertyValue(Ptr);
}

template<typename TScalar>
FORCEINLINE FDcResult ReadTopStateScalarProperty(FDcPropertyReader* Self, TScalar* OutPtr)
{
	using TProperty = typename DcPropertyUtils::TPropertyTypeMap<TScalar>::Type;

	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(Self).ReadDataEntry(Self, TProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		check(!Datum.Property.IsUObject())
		ReadPropertyValueConversion<TProperty, TScalar>(Datum.Property.ToFieldUnsafe(), Datum.DataPtr, OutPtr);
	}

	return DcOk();
}

FDcPropertyReader::FDcPropertyReader()
{
	Config = FDcPropertyConfig::MakeDefault();
	PushNoneState(this);
}

FDcPropertyReader::FDcPropertyReader(FDcPropertyDatum Datum)
	: FDcPropertyReader()
{
	if (Datum.IsNone())
	{
		//	pass
	}
	else if (Datum.Property.IsA<UClass>())
	{
		UObject* Obj = (UObject*)(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassPropertyState(
			this,
			Obj,
			Datum.CastUClassChecked(),
			FDcReadStateClass::EType::Root,
			Obj->GetFName()
		);
	}
	else if (Datum.Property.IsA<UScriptStruct>())
	{
		PushStructPropertyState(this, Datum.DataPtr, Datum.CastUScriptStructChecked(), FName(TEXT("$root")));
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
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	else if (Datum.Property.IsA<FOptionalProperty>())
	{
		PushOptionalPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FOptionalProperty>());
	}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
	else 
	{
		PushScalarPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FProperty>());
	}
}

FDcPropertyReader::FDcPropertyReader(EArrayReader, FProperty* InInnerProperty, void* InArray, EArrayPropertyFlags InArrayFlags)
	: FDcPropertyReader()
{
	PushArrayPropertyState(this, InInnerProperty, InArray, InArrayFlags);
}

FDcPropertyReader::FDcPropertyReader(ESetReader, FProperty* InElementProperty, void* InSet)
	: FDcPropertyReader()
{
	PushSetPropertyState(this, InElementProperty, InSet);
}

FDcPropertyReader::FDcPropertyReader(FProperty* InKeyProperty, FProperty* InValueProperty, void* InMap, EMapPropertyFlags InMapFlags)
	: FDcPropertyReader()
{
	PushMappingPropertyState(this, InKeyProperty, InValueProperty, InMap, InMapFlags);
}

FDcResult FDcPropertyReader::Coercion(EDcDataEntry ToEntry, bool* OutPtr)
{
	EDcDataEntry Next;
	if (!PeekRead(&Next).Ok())
		return ReadOutOk(OutPtr, false);

	if (Next == EDcDataEntry::ArrayRoot)
	{
		return ReadOutOk(OutPtr, ToEntry == EDcDataEntry::Blob);
	}
	else if (Next == EDcDataEntry::StructRoot)
	{
		return ReadOutOk(OutPtr, ToEntry == EDcDataEntry::Blob);
	}
	else
	{
		return ReadOutOk(OutPtr, false);
	}
}

FDcResult FDcPropertyReader::PeekRead(EDcDataEntry* OutPtr)
{
	return GetTopState(this).PeekRead(this, OutPtr);
}

FDcResult FDcPropertyReader::ReadBool(bool* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadString(FString* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadText(FText* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadEnum(FDcEnumData* OutPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).ReadDataEntry(this, FProperty::StaticClass(), Datum));
	
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;
	
	bool bIsEnum = DcPropertyUtils::IsEnumAndTryUnwrapEnum(Datum.Property, Enum, UnderlyingProperty);
	if (!bIsEnum)
		return DC_FAIL(DcDReadWrite, PropertyMismatch2)
			<< TEXT("EnumProperty")  << TEXT("<NumericProperty with Enum>")
			<< Datum.Property.GetFName() << Datum.Property.GetClassName();

	if (OutPtr)
	{
		bool bIsUnsigned = DcPropertyUtils::IsUnsignedProperty(UnderlyingProperty);
		OutPtr->bIsUnsigned = bIsUnsigned;
		if (bIsUnsigned)
			OutPtr->Unsigned64 = UnderlyingProperty->GetUnsignedIntPropertyValue(Datum.DataPtr);
		else
			OutPtr->Signed64 = UnderlyingProperty->GetSignedIntPropertyValue(Datum.DataPtr);

		if (Enum)
		{
			OutPtr->Type = Enum->GetFName();
			OutPtr->Name = Enum->GetNameByValue(OutPtr->Signed64);
		}
		else
		{
			OutPtr->Type = NAME_None;
			OutPtr->Name = NAME_None;
		}
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadName(FName* OutPtr)
{
	DC_TRY(GetTopState(this).ReadName(this, OutPtr));

	return DcOk();
}

FDcResult FDcPropertyReader::ReadStructRootAccess(FDcStructAccess& Access)
{
	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateStruct* StructState = TopState.As<FDcReadStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FDcReadStateStruct::EState::ExpectRoot)
		{
			DC_TRY(StructState->ReadStructRootAccess(this, Access));
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(this, FStructProperty::StaticClass(), Datum));

		FStructProperty* StructProperty = Datum.CastFieldChecked<FStructProperty>();
		FDcReadStateStruct& ChildStruct = PushStructPropertyState(
			this,
			Datum.DataPtr,
			StructProperty->Struct,
			StructProperty->GetFName()
		);
		DC_TRY(ChildStruct.ReadStructRootAccess(this, Access));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadStructEndAccess(FDcStructAccess& Access)
{
	if (FDcReadStateStruct* StructState = TryGetTopState<FDcReadStateStruct>(this))
	{
		DC_TRY(StructState->ReadStructEndAccess(this, Access));
		PopState<FDcReadStateStruct>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateStruct::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadClassRootAccess(FDcClassAccess& Access)
{
	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateClass* ClassState = TopState.As<FDcReadStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FDcReadStateClass::EState::ExpectRoot)
		{
			DC_TRY(ClassState->ReadClassRootAccess(this, Access));
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(this, FObjectProperty::StaticClass(), Datum));

		FObjectProperty* ObjProperty = Datum.CastFieldChecked<FObjectProperty>();
		check(ObjProperty);

		DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Datum.DataPtr));

		FDcReadStateClass::EType StateType;
		switch (Access.Control)
		{
			case FDcClassAccess::EControl::Default:
			{
				if (Config.ShouldExpandObject(ObjProperty))
				{
					Access.Control = FDcClassAccess::EControl::ExpandObject;
					StateType = FDcReadStateClass::EType::PropertyInstanced;
				}
				else
				{
					Access.Control = FDcClassAccess::EControl::ReferenceOrNone;
					StateType = FDcReadStateClass::EType::PropertyNormal;
				}
				break;
			}
			case FDcClassAccess::EControl::ReferenceOrNone:
				StateType = FDcReadStateClass::EType::PropertyNormal;
				break;
			case FDcClassAccess::EControl::ExpandObject:
				StateType = FDcReadStateClass::EType::PropertyInstanced;
				break;
			default:
				return DcNoEntry();
		}


		FDcReadStateClass& ChildClass = PushClassPropertyState(
			this,
			ObjProperty->GetObjectPropertyValue(Datum.DataPtr),
			ObjProperty->PropertyClass,
			StateType,
			ObjProperty->GetFName()
		);
		DC_TRY(ChildClass.ReadClassRootAccess(this, Access));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadClassEndAccess(FDcClassAccess& Access)
{
	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		DC_TRY(ClassState->ReadClassEndAccess(this, Access));
		PopState<FDcReadStateClass>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadMapRoot()
{
	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateMap* MapState = TopState.As<FDcReadStateMap>();
		if (MapState != nullptr
			&& MapState->State == FDcReadStateMap::EState::ExpectRoot)
		{
			return MapState->ReadMapRoot(this);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).ReadDataEntry(this, FMapProperty::StaticClass(), Datum));

		FDcReadStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FMapProperty>());
		DC_TRY(ChildMap.ReadMapRoot(this));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadMapEnd()
{
	if (FDcReadStateMap* StateMap = TryGetTopState<FDcReadStateMap>(this))
	{
		DC_TRY(StateMap->ReadMapEnd(this));
		PopState<FDcReadStateMap>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadArrayRoot()
{
	FDcBaseReadState& TopState = GetTopState(this);

	{
		if (FDcReadStateArray* ArrayState = TopState.As<FDcReadStateArray>())
		{
			if (ArrayState->State == FDcReadStateArray::EState::ExpectRoot)
				return ArrayState->ReadArrayRoot(this);
		}
		else if (FDcReadStateScalar* ScalarState = TopState.As<FDcReadStateScalar>())
		{
			if (ScalarState->State == FDcReadStateScalar::EState::ExpectArrayRoot)
				return ScalarState->ReadArrayRoot(this);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(this, FProperty::StaticClass(), Datum));

		if (Datum.IsA<FArrayProperty>())
		{
			FDcReadStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FArrayProperty>());
			DC_TRY(ChildArray.ReadArrayRoot(this));
		}
		else if (DcPropertyUtils::IsScalarArray(Datum.Property))
		{
			FDcReadStateScalar& ChildScalarArray = PushScalarArrayPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FProperty>());
			DC_TRY(ChildScalarArray.ReadArrayRoot(this));
		}
		else
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch2)
				<< TEXT("ArrayProperty") << TEXT("[Property w/ ArrayDim > 1]")
				<< Datum.Property.GetFName() << Datum.Property.GetClassName();
		}
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadArrayEnd()
{
	FDcBaseReadState& TopState = GetTopState(this);
	if (FDcReadStateArray* ArrayState = TopState.As<FDcReadStateArray>())
	{
		DC_TRY(ArrayState->ReadArrayEnd(this));
		PopState<FDcReadStateArray>(this);
		return DcOk();
	}
	else if (FDcReadStateScalar* ScalarState = TopState.As<FDcReadStateScalar>())
	{
		DC_TRY(ScalarState->ReadArrayEnd(this));
		PopState<FDcReadStateScalar>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateArray::ID << (int)TopState.GetType();
	}
}

FDcResult FDcPropertyReader::ReadSetRoot()
{
	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateSet* SetState = TopState.As<FDcReadStateSet>();
		if (SetState != nullptr
			&& SetState->State == FDcReadStateSet::EState::ExpectRoot)
		{
			return SetState->ReadSetRoot(this);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(this, FSetProperty::StaticClass(), Datum));

		FDcReadStateSet& ChildSet = PushSetPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FSetProperty>());
		DC_TRY(ChildSet.ReadSetRoot(this));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadSetEnd()
{
	if (FDcReadStateSet* SetState = TryGetTopState<FDcReadStateSet>(this))
	{
		DC_TRY(SetState->ReadSetEnd(this));
		PopState<FDcReadStateSet>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateArray::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadOptionalRoot()
{
#if UE_VERSION_OLDER_THAN(5, 4, 0)
	return DC_FAIL(DcDReadWrite, PropertyNotSupportedUEVersion)
		<< TEXT("Optional Property");
#else
	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateOptional* OptionalState = TopState.As<FDcReadStateOptional>();
		if (OptionalState != nullptr
			&& OptionalState->State == FDcReadStateOptional::EState::ExpectRoot)
		{
			return OptionalState->ReadOptionalRoot(this);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(this, FOptionalProperty::StaticClass(), Datum));

		FDcReadStateOptional& ChildSet = PushOptionalPropertyState(this, Datum.DataPtr, Datum.CastFieldChecked<FOptionalProperty>());
		DC_TRY(ChildSet.ReadOptionalRoot(this));
	}

	return DcOk();
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
}

FDcResult FDcPropertyReader::ReadOptionalEnd()
{
#if UE_VERSION_OLDER_THAN(5, 4, 0)
	return DC_FAIL(DcDReadWrite, PropertyNotSupportedUEVersion)
		<< TEXT("Optional Property");
#else
	if (FDcReadStateOptional* OptionalState = TryGetTopState<FDcReadStateOptional>(this))
	{
		DC_TRY(OptionalState->ReadOptionalEnd(this));
		PopState<FDcReadStateOptional>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateOptional::ID << (int)GetTopState(this).GetType();
	}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
}

FDcResult FDcPropertyReader::ReadObjectReference(UObject** OutPtr)
{
	//	only class property reads reference
	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		return ClassState->ReadObjectReference(this, OutPtr);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();

	}
}

FDcResult FDcPropertyReader::ReadClassReference(UClass** OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadWeakObjectReference(FWeakObjectPtr* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadLazyObjectReference(FLazyObjectPtr* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInterfaceReference(FScriptInterface* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadSoftObjectReference(FSoftObjectPtr* OutPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).ReadDataEntry(this, FSoftObjectProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		FSoftObjectProperty* SoftObjectProperty = Datum.CastFieldChecked<FSoftObjectProperty>();
		*OutPtr = SoftObjectProperty->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadSoftClassReference(FSoftObjectPtr* OutPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).ReadDataEntry(this, FSoftClassProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		FSoftClassProperty* SoftClassProperty = Datum.CastFieldChecked<FSoftClassProperty>();
		*OutPtr = SoftClassProperty->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadFieldPath(FFieldPath* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadDelegate(FScriptDelegate* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadMulticastSparseDelegate(FMulticastScriptDelegate* OutPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).ReadDataEntry(this, FMulticastSparseDelegateProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		FMulticastSparseDelegateProperty* SparseProperty = Datum.CastFieldChecked<FMulticastSparseDelegateProperty>();
		const FMulticastScriptDelegate* DelegatePtr = SparseProperty->GetMulticastDelegate(Datum.DataPtr);

		if (DelegatePtr)
			*OutPtr = *DelegatePtr;
		else
			OutPtr->Clear();
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadInt8(int8* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt16(int16* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt32(int32* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt64(int64* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadUInt8(uint8* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt16(uint16* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt32(uint32* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt64(uint64* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadFloat(float* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadDouble(double* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadBlob(FDcBlobViewData* OutPtr)
{
	FFieldVariant NextProperty;
	DC_TRY(GetTopState(this).PeekReadProperty(this, &NextProperty));

	FProperty* Prop =  CastField<FProperty>(NextProperty.ToField());
	if (Prop == nullptr)
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::ArrayRoot << EDcDataEntry::StructRoot
			<< DcPropertyUtils::PropertyToDataEntry(NextProperty);

	if (Prop->IsA<FArrayProperty>())
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).ReadDataEntry(this, FArrayProperty::StaticClass(), Datum));

		FArrayProperty* ArrayProperty = Datum.CastFieldChecked<FArrayProperty>();
		FScriptArrayHelper ScriptArray(ArrayProperty, Datum.DataPtr);

		if (OutPtr)
		{
			*OutPtr = {
				(uint8*)ScriptArray.GetRawPtr(),
				ScriptArray.Num() * ArrayProperty->Inner->ElementSize
			};
		}

		return DcOk();
	}
	else if (Prop->ArrayDim > 1)
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).ReadDataEntry(this, FStructProperty::StaticClass(), Datum));

		if (OutPtr)
		{
			*OutPtr = {
				(uint8*)Datum.DataPtr,
				Prop->ArrayDim * Prop->ElementSize
			};
		}

		return DcOk();
	}
	else if (Prop->IsA<FStructProperty>())
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).ReadDataEntry(this, FStructProperty::StaticClass(), Datum));

		FStructProperty* StructProperty = Datum.CastFieldChecked<FStructProperty>();
		if (OutPtr)
		{
			*OutPtr = {
				(uint8*)Datum.DataPtr,
				StructProperty->ElementSize
			};
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
			<< EDcDataEntry::ArrayRoot << EDcDataEntry::StructRoot
			<< DcPropertyUtils::PropertyToDataEntry(NextProperty);
	}
}

FDcResult FDcPropertyReader::SkipRead()
{
	return GetTopState(this).SkipRead(this);
}

FDcResult FDcPropertyReader::PeekReadProperty(FFieldVariant* OutProperty)
{
	return GetTopState(this).PeekReadProperty(this, OutProperty);
}

FDcResult FDcPropertyReader::PeekReadDataPtr(void** OutDataPtr)
{
	return GetTopState(this).PeekReadDataPtr(this, OutDataPtr);
}

FDcResult FDcPropertyReader::ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	return GetTopState(this).ReadDataEntry(this, ExpectedPropertyClass, OutDatum);
}

FDcResult FDcPropertyReader::PushTopClassPropertyState(const FDcPropertyDatum& Datum)
{
	if (FObjectProperty* ObjProperty = Datum.CastField<FObjectProperty>())
	{
		PushClassPropertyState(
			this,
			ObjProperty->GetObjectPropertyValue(Datum.DataPtr),
			ObjProperty->PropertyClass,
			FDcReadStateClass::EType::Root,
			ObjProperty->GetFName()
		);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("ObjectProperty") << Datum.Property.GetFName() << Datum.Property.GetClassName();
	}
}

FDcResult FDcPropertyReader::PushTopStructPropertyState(const FDcPropertyDatum& Datum, const FName& StructName)
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

bool FDcPropertyReader::IsReadingScalarArrayItem()
{
	if (FDcReadStateScalar* ScalarState = GetTopState(this).As<FDcReadStateScalar>())
		return ScalarState->State == FDcReadStateScalar::EState::ExpectArrayItem;

	return false;
}

FDcResult FDcPropertyReader::SetConfig(FDcPropertyConfig InConfig)
{
	Config = MoveTemp(InConfig);
	return Config.Prepare();
}

FDcResult FDcPropertyReader::ReadNone()
{
	//	only class/optional property accepts none
	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		return ClassState->ReadNone(this);
	}
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	else if (FDcReadStateOptional* OptionalState = TryGetTopState<FDcReadStateOptional>(this))
	{
		return OptionalState->ReadNone(this);
	}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcDiagnosticHighlight FDcPropertyReader::FormatHighlight()
{
	FDcDiagnosticHighlight OutHighlight(this, ClassId().ToString());
	TArray<FString> Segments;

	bool bLastIsContainer = false;
	int Num = States.Num();
	for (int Ix = 1; Ix < Num; Ix++)
	{
		FDcBaseReadState& ReadState = AsReadState(&States[Ix].ImplStorage);
		DcPropertyHighlight::EFormatSeg Seg;
		if (bLastIsContainer)
			Seg = DcPropertyHighlight::EFormatSeg::ParentIsContainer;
		else if (Ix == Num - 1)
			Seg = DcPropertyHighlight::EFormatSeg::Last;
		else
			Seg = DcPropertyHighlight::EFormatSeg::Normal;

		ReadState.FormatHighlightSegment(Segments, Seg);

		EDcPropertyReadType StateType = ReadState.GetType();
		bLastIsContainer = StateType == EDcPropertyReadType::ArrayProperty
			|| StateType == EDcPropertyReadType::SetProperty
			|| StateType == EDcPropertyReadType::MapProperty;
	}

	FString Path = Segments.Num() == 0
		? TEXT("<none>")
		: FString::Join(Segments, TEXT("."));
	OutHighlight.Formatted = FString::Printf(TEXT("Reading property: %s"), *Path);
	return OutHighlight;
}

void FDcPropertyReader::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Diag << FormatHighlight();
}

FName FDcPropertyReader::ClassId() { return FName(TEXT("DcPropertyReader")); }
FName FDcPropertyReader::GetId() { return ClassId(); }

