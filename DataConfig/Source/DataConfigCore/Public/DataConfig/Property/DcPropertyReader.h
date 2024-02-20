#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcPropertyReaderDetails
{
	struct FReadState
	{
		using ImplStorageType = TAlignedBytes<96, MIN_ALIGNMENT>;
		ImplStorageType ImplStorage;
	};
} // namespace DcPropertyReaderDetails


struct DATACONFIGCORE_API FDcPropertyReader : public FDcReader, private FNoncopyable
{
	FDcPropertyReader();
	FDcPropertyReader(FDcPropertyDatum Datum);

	enum EArrayReader { Array };
	enum ESetReader { Set };

	FDcPropertyReader(EArrayReader, FProperty* InInnerProperty, void* InArray, EArrayPropertyFlags InArrayFlags = EArrayPropertyFlags::None);
	FDcPropertyReader(ESetReader, FProperty* InElementProperty, void* InSet);
	FDcPropertyReader(FProperty* InKeyProperty, FProperty* InValueProperty, void* InMap, EMapPropertyFlags InMapFlags = EMapPropertyFlags::None);

	FDcResult Coercion(EDcDataEntry ToEntry, bool* OutPtr) override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;

	FDcResult ReadNone() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadText(FText* OutPtr) override;
	FDcResult ReadEnum(FDcEnumData* OutPtr) override;

	FDcResult ReadStructRootAccess(FDcStructAccess& Access) override;
	FDcResult ReadStructEndAccess(FDcStructAccess& Access) override;

	FDcResult ReadClassRootAccess(FDcClassAccess& Access) override;
	FDcResult ReadClassEndAccess(FDcClassAccess& Access) override;

	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;

	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;

	FDcResult ReadSetRoot() override;
	FDcResult ReadSetEnd() override;

	FDcResult ReadOptionalRoot() override;
	FDcResult ReadOptionalEnd() override;

	FDcResult ReadObjectReference(UObject** OutPtr) override;
	FDcResult ReadClassReference(UClass** OutPtr) override;

	FDcResult ReadWeakObjectReference(FWeakObjectPtr* OutPtr) override;
	FDcResult ReadLazyObjectReference(FLazyObjectPtr* OutPtr) override;
	FDcResult ReadSoftObjectReference(FSoftObjectPtr* OutPtr) override;
	FDcResult ReadSoftClassReference(FSoftObjectPtr* OutPtr) override;
	FDcResult ReadInterfaceReference(FScriptInterface* OutPtr) override;

	FDcResult ReadFieldPath(FFieldPath* OutPtr) override;
	FDcResult ReadDelegate(FScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr) override;
	FDcResult ReadMulticastSparseDelegate(FMulticastScriptDelegate* OutPtr) override;

	FDcResult ReadInt8(int8* OutPtr) override;
	FDcResult ReadInt16(int16* OutPtr) override;
	FDcResult ReadInt32(int32* OutPtr) override;
	FDcResult ReadInt64(int64* OutPtr) override;

	FDcResult ReadUInt8(uint8* OutPtr) override;
	FDcResult ReadUInt16(uint16* OutPtr) override;
	FDcResult ReadUInt32(uint32* OutPtr) override;
	FDcResult ReadUInt64(uint64* OutPtr) override;

	FDcResult ReadFloat(float* OutPtr) override;
	FDcResult ReadDouble(double* OutPtr) override;
	FDcResult ReadBlob(FDcBlobViewData* OutPtr) override;

	///	try skip read at current position
	FDcResult SkipRead();
	///	peek next write property
	FDcResult PeekReadProperty(FFieldVariant* OutProperty);
	FDcResult PeekReadDataPtr(void** OutDataPtr);
	///	manual reading
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);

	///	manual writing supporting
	FDcResult PushTopClassPropertyState(const FDcPropertyDatum& Datum);
	FDcResult PushTopStructPropertyState(const FDcPropertyDatum& Datum, const FName& StructName);

	/// scalar array support
	bool IsReadingScalarArrayItem();

	FDcResult SetConfig(FDcPropertyConfig InConfig);
	FDcPropertyConfig Config;

	TArray<DcPropertyReaderDetails::FReadState, TInlineAllocator<4>> States;

	FDcDiagnosticHighlight FormatHighlight();
	void FormatDiagnostic(FDcDiagnostic& Diag) override;

	static FName ClassId();
	FName GetId() override;
};

template<> struct TIsPODType<DcPropertyReaderDetails::FReadState> { enum { Value = true }; };

