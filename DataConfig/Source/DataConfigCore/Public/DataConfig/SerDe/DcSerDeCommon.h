#pragma once

#include "CoreMinimal.h"


namespace DcSerDeCommon
{

struct FWeakObjectPtrAccess
{
	int32 ObjectIndex;
	int32 ObjectSerialNumber;
};
static_assert(sizeof(FWeakObjectPtrAccess) == sizeof(FWeakObjectPtr), "FWeakObjectPtrAccess stale");

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

struct FTextAccess
{
	TSharedRef<ITextData, ESPMode::ThreadSafe> TextData;
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

