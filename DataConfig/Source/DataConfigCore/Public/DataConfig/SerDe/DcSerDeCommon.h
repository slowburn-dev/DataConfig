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

struct FScriptDelegateAccess
{
	FWeakObjectPtr Object;
	FName FunctionName;
};
static_assert(sizeof(FScriptDelegateAccess) == sizeof(FScriptDelegate), "FScriptDelegateAccess stale");

struct FMulticastScriptDelegateAccess
{
	FMulticastScriptDelegate::FInvocationList InvocationList;
};
static_assert(sizeof(FMulticastScriptDelegateAccess) == sizeof(FMulticastScriptDelegate), "FMulticastScriptDelegateAccess stale");

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

