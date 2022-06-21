
template<typename TEnum>
FDcResult FDcWriter::WriteEnumField(const TEnum& Value)
{
	static_assert(TIsEnum<TEnum>::Value, "expect TEnum is a enum");
	FDcEnumData EnumData;
	EnumData.Unsigned64 = (uint64)Value;
	return WriteEnum(EnumData);
}

FORCEINLINE FDcResult FDcWriter::WriteStructRoot()
{
	FDcStructAccess Access;
	return WriteStructRootAccess(Access);
}

FORCEINLINE FDcResult FDcWriter::WriteStructEnd()
{
	FDcStructAccess Access;
	return WriteStructEndAccess(Access);
}

FORCEINLINE FDcResult FDcWriter::WriteClassRoot()
{
	FDcClassAccess Access;
	return WriteClassRootAccess(Access);
}

FORCEINLINE FDcResult FDcWriter::WriteClassEnd()
{
	FDcClassAccess Access;
	return WriteClassEndAccess(Access);
}

template<typename T>
T* FDcWriter::CastById()
{
	static_assert(TIsDerivedFrom<T, FDcWriter>::Value, "casting non writer");
	if (T::ClassId() == GetId())
		return (T*)(this);
	return nullptr;
}

template<typename T>
T* FDcWriter::CastByIdChecked()
{
	static_assert(TIsDerivedFrom<T, FDcWriter>::Value, "casting non writer");
	check(T::ClassId() == GetId());
	return (T*)(this);
}

template<typename TObject>
FDcResult FDcWriter::WriteWeakObjectField(const TWeakObjectPtr<TObject>& Value)
{
	static_assert(sizeof(FWeakObjectPtr) == sizeof(TWeakObjectPtr<TObject>), "TWeakkObjectPtr should have same memory layout as FWeakObjectPtr");

	const FWeakObjectPtr& WeakPtr = (const FWeakObjectPtr&)Value;
	return WriteWeakObjectReference(WeakPtr);
}

template<typename TObject>
FDcResult FDcWriter::WriteLazyObjectField(const TLazyObjectPtr<TObject>& Value)
{
	//	LazyPtr is missing a copy constructor, can only do assign
	FLazyObjectPtr LazyPtr;
	LazyPtr = Value.GetUniqueID();
	return WriteLazyObjectReference(LazyPtr);
}

template<typename TObject>
FDcResult FDcWriter::WriteSoftObjectField(const TSoftObjectPtr<TObject>& Value)
{
	static_assert(sizeof(TSoftObjectPtr<TObject>) == sizeof(FSoftObjectPtr), "TSoftObjectPtr<T> and FSoftObjectPtr should have identical layout");
	const FSoftObjectPtr& SoftObjectPtr = (const FSoftObjectPtr&)(Value);
	return WriteSoftObjectReference(SoftObjectPtr);
}

template<typename TClass>
FDcResult FDcWriter::WriteSoftClassField(const TSoftClassPtr<TClass>& Value)
{
	static_assert(sizeof(TSoftClassPtr<TClass>) == sizeof(FSoftObjectPtr), "TSoftClassPtr<T> and FSoftObjectPtr should have identical layout");
	const FSoftObjectPtr& SoftObjectPtr = (const FSoftObjectPtr&)(Value);
	return WriteSoftClassReference(SoftObjectPtr);
}

template<typename TInterface>
FDcResult FDcWriter::WriteInterfaceField(const TScriptInterface<TInterface>& Value)
{
	FScriptInterface ScriptInterface(Value);
	return WriteInterfaceReference(ScriptInterface);
}

template<typename TProperty>
FDcResult FDcWriter::WritePropertyField(const TFieldPath<TProperty>& Value)
{
	static_assert(sizeof(FFieldPath) == sizeof(TFieldPath<TProperty>), "FFieldPath and TFieldPath should have identical layout");

	const FFieldPath& FieldPath = (const FFieldPath&)Value;
	return WriteFieldPath(FieldPath);
}

namespace DcPropertyUtils
{
	DATACONFIGCORE_API FDcResult FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset, FProperty*& OutValue);
} // namespace DcPropertyUtils

template<typename MulticastDelegate, typename OwningClass, typename DelegateInfoClass>
FDcResult FDcWriter::WriteSparseDelegateField(const TSparseDynamicDelegate<MulticastDelegate, OwningClass, DelegateInfoClass>& Value)
{
	FProperty* Property;
	DC_TRY(DcPropertyUtils::FindEffectivePropertyByOffset(
		OwningClass::StaticClass(),
		DelegateInfoClass::template GetDelegateOffset<OwningClass>(),
		Property
	));

	FMulticastSparseDelegateProperty* SparseProperty = CastFieldChecked<FMulticastSparseDelegateProperty>(Property);
	const FMulticastScriptDelegate* Result = SparseProperty->GetMulticastDelegate(&Value);

	if (Result == nullptr)
	{
		FMulticastScriptDelegate Empty;
		return WriteMulticastSparseDelegate(Empty);
	}
	else
	{
		return WriteMulticastSparseDelegate(*Result);
	}
}

#if ENGINE_MAJOR_VERSION == 5
template <typename TObject>
FDcResult FDcWriter::WriteTObjectPtr(const TObjectPtr<TObject>& Value)
{
	return WriteObjectReference(Value.Get());
}
#endif
