#pragma once

#include "CoreMinimal.h"

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
#include "Internationalization/ITextData.h"
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

namespace DcSerDeCommon
{

struct FWeakObjectPtrAccess
{
	int32 ObjectIndex;
	int32 ObjectSerialNumber;
};
static_assert(sizeof(FWeakObjectPtrAccess) == sizeof(FWeakObjectPtr), "FWeakObjectPtrAccess stale");


#if UE_VERSION_OLDER_THAN(5, 8, 0)

template<typename TDelegate>
struct TDelegateAccess : public TDelegate
{
	FORCEINLINE FWeakObjectPtr& GetObject() { return this->Object; }
	FORCEINLINE FName& GetFunctionName() { return this->FunctionName; }
};
using FScriptDelegateAccess = TDelegateAccess<FScriptDelegate>;

struct FMulticastScriptDelegateAccess : public FMulticastScriptDelegate
{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	using TypeInvocationList = FMulticastScriptDelegate::FInvocationList;
#else
	using TypeInvocationList = FMulticastScriptDelegate::InvocationListType;
#endif

	using TypeInvocationAccess = TDelegateAccess<typename TypeInvocationList::ElementType>;

	FORCEINLINE TypeInvocationList& GetInvocationList() { return InvocationList; }
};

#else // UE_VERSION_OLDER_THAN(5, 8, 0)

template<typename ThreadSafetyMode = FNotThreadSafeDelegateMode>
struct TScriptDelegateAccess : public TDelegateAccessHandlerBase<ThreadSafetyMode>
{
	FWeakObjectPtr Object;
	FName FunctionName;

#if defined(UE_USE_DYNAMIC_DELEGATE_PAYLOADS) && UE_USE_DYNAMIC_DELEGATE_PAYLOADS
	UPTRINT UnknownPayload[2];
#endif

	FORCEINLINE FWeakObjectPtr& GetObject() { return this->Object; }
	FORCEINLINE FName& GetFunctionName() { return this->FunctionName; }
};
using FScriptDelegateAccess = TScriptDelegateAccess<>;
static_assert(sizeof(FScriptDelegate) == sizeof(FScriptDelegateAccess), "FScriptDelegate stale");

struct FMulticastScriptDelegateAccess : public FMulticastScriptDelegate
{
	using TypeInvocationAccess = TScriptDelegateAccess<FNotThreadSafeNotCheckedDelegateMode>;

	FORCEINLINE auto& GetInvocationList() { return InvocationList; }
};
#endif // UE_VERSION_OLDER_THAN(5, 8, 0)

struct FTextAccess
{
#if UE_VERSION_OLDER_THAN(5, 4, 0)
	TSharedRef<ITextData, ESPMode::ThreadSafe> TextData;
#else
	TRefCountPtr<ITextData> TextData;
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
	uint32 Flags;

};
static_assert(sizeof(FTextAccess) == sizeof(FText), "FTextAccess stale");

struct FSharedRefAccess
{
	void* Object;
	void* SharedReferenceCount;
};
static_assert(sizeof(FSharedRefAccess) == sizeof(TSharedRef<FTextAccess>), "FSharedRefAccess stale");

struct FScriptInterfaceAccess
{
	UObject*	ObjectPointer;
	void*		InterfacePointer;
};
static_assert(sizeof(FScriptInterfaceAccess) == sizeof(FScriptInterface), "FScriptInterfaceAccess stale");

struct FScriptArrayHelperAccess
{
	FProperty* InnerProperty;
	union
	{
		FScriptArray* HeapArray;
		FFreezableScriptArray* FreezableArray;
	};
	int32 ElementSize;
#if ENGINE_MAJOR_VERSION == 5
	uint32 ElementAlignment;
#endif
	EArrayPropertyFlags ArrayFlags;
};
static_assert(sizeof(FScriptArrayHelper) == sizeof(FScriptArrayHelperAccess), "FScriptArrayHelperAccess stale");

} // namespace DcSerDeCommon

