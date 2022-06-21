
template<typename TEnum>
FDcResult FDcReader::ReadEnumField(TEnum* OutPtr)
{
	static_assert(TIsEnum<TEnum>::Value, "expect TEnum is a enum");
	FDcEnumData Value;
	DC_TRY(ReadEnum(&Value));
	ReadOut(OutPtr, (TEnum)(Value.Unsigned64));
	return DcOk();
}

FORCEINLINE FDcResult FDcReader::ReadStructRoot()
{
	FDcStructAccess Access;
	return ReadStructRootAccess(Access);
}

FORCEINLINE FDcResult FDcReader::ReadStructEnd()
{
	FDcStructAccess Access;
	return ReadStructEndAccess(Access);
}

FORCEINLINE FDcResult FDcReader::ReadClassRoot()
{
	FDcClassAccess Access;
	return ReadClassRootAccess(Access);
}

FORCEINLINE FDcResult FDcReader::ReadClassEnd()
{
	FDcClassAccess Access;
	return ReadClassEndAccess(Access);
}

template <typename T>
T* FDcReader::CastById()
{
	static_assert(TIsDerivedFrom<T, FDcReader>::Value, "casting non reader");
	if (T::ClassId() == GetId())
		return (T*)(this);
	return nullptr;
}

template <typename T>
T* FDcReader::CastByIdChecked()
{
	static_assert(TIsDerivedFrom<T, FDcReader>::Value, "casting non reader");
	check(T::ClassId() == GetId());
	return (T*)(this);
}

template<typename TObject>
FDcResult FDcReader::ReadWeakObjectField(TWeakObjectPtr<TObject>* OutPtr)
{
	static_assert(sizeof(FWeakObjectPtr) == sizeof(TWeakObjectPtr<TObject>), "TWeakkObjectPtr should have same memory layout as FWeakObjectPtr");

	FWeakObjectPtr* WeakOutPtr = (FWeakObjectPtr*)OutPtr;
	return ReadWeakObjectReference(WeakOutPtr);
}

template<typename TObject>
FDcResult FDcReader::ReadLazyObjectField(TLazyObjectPtr<TObject>* OutPtr)
{
	FLazyObjectPtr LazyPtr;
	DC_TRY(ReadLazyObjectReference(&LazyPtr));

	ReadOut(OutPtr, LazyPtr.GetUniqueID());
	return DcOk();
}

template<typename TObject>
FDcResult FDcReader::ReadSoftObjectField(TSoftObjectPtr<TObject>* OutPtr)
{
	static_assert(sizeof(TSoftObjectPtr<TObject>) == sizeof(FSoftObjectPtr), "TSoftObjectPtr<T> and FSoftObjectPtr should have identical layout");

	FSoftObjectPtr* SoftObjectPtr = (FSoftObjectPtr*)OutPtr;
	return ReadSoftObjectReference(SoftObjectPtr);
}

template<typename TClass>
FDcResult FDcReader::ReadSoftClassField(TSoftClassPtr<TClass>* OutPtr)
{
	static_assert(sizeof(TSoftClassPtr<TClass>) == sizeof(FSoftObjectPtr), "TSoftClassPtr<T> and FSoftObjectPtr should have identical layout");

	FSoftObjectPtr* SoftObjectPtr = (FSoftObjectPtr*)OutPtr;
	return ReadSoftClassReference(SoftObjectPtr);
}

template<typename TInterface>
FDcResult FDcReader::ReadInterfaceField(TScriptInterface<TInterface>* OutPtr)
{
	FScriptInterface ScriptInterface;
	DC_TRY(ReadInterfaceReference(OutPtr));

	ReadOut(OutPtr, (const TScriptInterface<TInterface>&)ScriptInterface);
	return DcOk();
}

template<typename TProperty>
FDcResult FDcReader::ReadPropertyField(TFieldPath<TProperty>* OutPtr)
{
	static_assert(sizeof(FFieldPath) == sizeof(TFieldPath<TProperty>), "FFieldPath and TFieldPath should have identical layout");
	FFieldPath* FieldPathPtr = (FFieldPath*)OutPtr;
	return ReadFieldPath(FieldPathPtr);
}

namespace DcPropertyUtils
{
DATACONFIGCORE_API FDcResult FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset, FProperty*& OutValue);
} // namespace DcPropertyUtils

template<typename MulticastDelegate, typename OwningClass, typename DelegateInfoClass>
FDcResult FDcReader::ReadSparseDelegateField(TSparseDynamicDelegate<MulticastDelegate, OwningClass, DelegateInfoClass>* OutPtr)
{
	FMulticastScriptDelegate Value;
	DC_TRY(ReadMulticastSparseDelegate(&Value));

	if (OutPtr)
	{
		FProperty* Property;
		DC_TRY(DcPropertyUtils::FindEffectivePropertyByOffset(
			OwningClass::StaticClass(), 
			DelegateInfoClass::template GetDelegateOffset<OwningClass>(),
			Property
		));

		FMulticastSparseDelegateProperty* SparseProperty = CastFieldChecked<FMulticastSparseDelegateProperty>(Property);
		SparseProperty->SetMulticastDelegate(OutPtr, Value);
	}

	return DcOk();
}

#if ENGINE_MAJOR_VERSION == 5
template <typename TObject>
FDcResult FDcReader::ReadTObjectPtr(TObjectPtr<TObject>* OutPtr)
{
	UObject* Obj;
	DC_TRY(ReadObjectReference(&Obj));

	TObject* TObj = ::CastChecked<TObject>(Obj);
	ReadOut(OutPtr, TObj);
	return DcOk();
}
#endif

