#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty.h"
#include "DcTestSerDe.h"
#include "DcTestRoundtrip.generated.h"

USTRUCT()
struct FDcTestRoundtripStruct1
{
	GENERATED_BODY()

	//	FieldPath
	UPROPERTY() TFieldPath<FNameProperty> FieldPathField;
	UPROPERTY() TFieldPath<FNameProperty> FieldPathField2;

	void MakeFixture();
};

UCLASS()
class UDcTestRoundtrip1 : public UObject
{
	GENERATED_BODY()

public:
	
	//	Primitives
	UPROPERTY() FDcTestStruct1 StructPrimitives;

	//	Struct Flags
	UPROPERTY() FDcTestStructEnumFlag1 StructEnumFlag;

	//	Containers
 	UPROPERTY() FDcTestStruct3 StructContainers;

	//	Others
	UPROPERTY() FDcTestRoundtripStruct1 StructOthers;

	//	Inline Subs
	UPROPERTY() FDcTestStructShapeContainer1 StructInlineSub;

	//	Object Refs
	UPROPERTY() FDcTestStructRefs1 StructObjectRefs;

	//	Class Refs;
	UPROPERTY() FDcTestStructRefs2 StructClassRefs;

};


class UDcTestRoundtrip2_Transient;

//	note that sparse callback is bound to a UClass
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FDcTestRoundtrip2SparseCallback, UDcTestRoundtrip2_Transient, SparseField, int, Value);

UCLASS()
class UDcTestRoundtrip2_Transient : public UObject
{
	GENERATED_BODY()

public:
	//	Delegates
	UPROPERTY() FDcTestDelegate1 DelegateField;
	UPROPERTY() FDcTestDelegate1 DelegateField2;

	UPROPERTY() FDcTestDynMulticastCallback1 MulticastField;
	UPROPERTY() FDcTestDynMulticastCallback1 MulticastField2;

	UPROPERTY() FDcTestRoundtrip2SparseCallback SparseField;
	UPROPERTY() FDcTestRoundtrip2SparseCallback SparseField2;

	//	Interface
	UPROPERTY() TScriptInterface<IDcTestInterface1> InterfaceField;
	UPROPERTY() TScriptInterface<IDcTestInterface1> InterfaceField2;

	void MakeFixture();
};
