#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcWriter
{
	FDcWriter();
	virtual ~FDcWriter();

	virtual FDcResult WriteNext(EDcDataEntry Next);

	virtual FDcResult WriteNil();
	virtual FDcResult WriteBool(bool Value);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteString(const FString& Value);
	virtual FDcResult WriteText(const FText& Value);
	virtual FDcResult WriteEnum(const FDcEnumData& Value);

	virtual FDcResult WriteStructRoot(const FName& Name);
	virtual FDcResult WriteStructEnd(const FName& Name);

	virtual FDcResult WriteClassRoot(const FDcClassPropertyStat& Class);
	virtual FDcResult WriteClassEnd(const FDcClassPropertyStat& Class);

	virtual FDcResult WriteMapRoot();
	virtual FDcResult WriteMapEnd();

	virtual FDcResult WriteArrayRoot();
	virtual FDcResult WriteArrayEnd();

	virtual FDcResult WriteSetRoot();
	virtual FDcResult WriteSetEnd();

	virtual FDcResult WriteObjectReference(const UObject* Value);
	virtual FDcResult WriteClassReference(const UClass* Value);

	virtual FDcResult WriteInt8(const int8& Value);
	virtual FDcResult WriteInt16(const int16& Value);
	virtual FDcResult WriteInt32(const int32& Value);
	virtual FDcResult WriteInt64(const int64& Value);

	virtual FDcResult WriteUInt8(const uint8& Value);
	virtual FDcResult WriteUInt16(const uint16& Value);
	virtual FDcResult WriteUInt32(const uint32& Value);
	virtual FDcResult WriteUInt64(const uint64& Value);

	virtual FDcResult WriteFloat(const float& Value);
	virtual FDcResult WriteDouble(const double& Value);
};


