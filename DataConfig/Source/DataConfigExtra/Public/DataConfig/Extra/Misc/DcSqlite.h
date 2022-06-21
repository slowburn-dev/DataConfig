#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DcSqlite.generated.h"

class FSQLiteDatabase;

namespace DcExtra
{

///	Load array of struct from Sqlite query
FDcResult LoadStructArrayFromSQLite(FSQLiteDatabase* Db, const TCHAR* Query, FDcPropertyDatum Datum);

template<typename TStruct>
FDcResult LoadStructArrayFromSQLite(FSQLiteDatabase* Db, const TCHAR* Query, TArray<TStruct>& Arr)
{
	using namespace DcPropertyUtils;
	auto ArrProp = FDcPropertyBuilder::Array(
		FDcPropertyBuilder::Struct(TBaseStructure<TStruct>::Get())
	).LinkOnScope();

	return LoadStructArrayFromSQLite(Db, Query, FDcPropertyDatum(ArrProp.Get(), &Arr));
}

} // namespace DcExtra

USTRUCT()
struct FDcExtraTestUser
{
	GENERATED_BODY()

	UPROPERTY() int Id;
	UPROPERTY() FString Name;
	UPROPERTY() FName Title;
};




