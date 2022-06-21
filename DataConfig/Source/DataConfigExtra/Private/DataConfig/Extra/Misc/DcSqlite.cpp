#include "DataConfig/Extra/Misc/DcSqlite.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Json/DcJsonWriter.h"

#include "SQLiteDatabase.h"
#include "Misc/ScopeExit.h"

namespace DcExtra
{

namespace SqliteDetails
{

static TOptional<FDcDeserializer> Deserializer;
static void LazyInitializeDeserializer()
{
	if (Deserializer.IsSet())
		return;
	
	Deserializer.Emplace();

	using namespace DcCommonHandlers;
	AddNumericPipeDirectHandlers(*Deserializer);

	Deserializer->AddDirectHandler(FNameProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeNameDeserialize));
	Deserializer->AddDirectHandler(FStrProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeStringDeserialize));
	Deserializer->AddDirectHandler(FTextProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerPipeTextDeserialize));

	Deserializer->AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
	Deserializer->AddDirectHandler(FStructProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapToStructDeserialize));
}

static EDcDataEntry SqliteColumnTypeToDataEntry(ESQLiteColumnType ColType)
{
	switch (ColType)
	{
	case ESQLiteColumnType::Null: return EDcDataEntry::Nil;
	case ESQLiteColumnType::String: return EDcDataEntry::String;
	case ESQLiteColumnType::Integer: return EDcDataEntry::Int32;
	case ESQLiteColumnType::Float: return EDcDataEntry::Float;
	case ESQLiteColumnType::Blob: return EDcDataEntry::Blob;
	}
	return EDcDataEntry::Ended;
}

struct FSqliteReader : FDcReader
{
	enum class EState
	{
		ExpectArrayRoot,
		ExpectArrayEnd,

		ExpectMapRoot,
		ExpectMapEnd,

		ExpectKey,
		ExpectValue,

		Ended,
	};

	EState State = EState::ExpectArrayRoot;
	int ColIx = 0;

	FSQLiteDatabase* Db;
	FSQLitePreparedStatement* Stmt;

	bool bPrepared = false;;

	TArray<FString> Names;
	TArray<ESQLiteColumnType> Types;

	FSqliteReader(FSQLiteDatabase* InDb, FSQLitePreparedStatement* InStmt)
	{
		Stmt = InStmt;
		Db = InDb;
	}

	FDcResult CacheNameAndTypes()
	{
		if (!bPrepared)
		{
			Names = Stmt->GetColumnNames();
			for (int Ix = 0; Ix < Names.Num(); Ix++)
			{
				ESQLiteColumnType ColType;
				if (!Stmt->GetColumnTypeByIndex(Ix, ColType))
					return DC_FAIL(DcDExtra, SqliteLastError)
						<< Db->GetLastError();

				Types.Add(ColType);
			}
			
			bPrepared = true;
		}

		return DcOk();
	}

	FDcResult Step()
	{
		ESQLitePreparedStatementStepResult Ret = Stmt->Step();
		if (Ret == ESQLitePreparedStatementStepResult::Row)
		{
			DC_TRY(CacheNameAndTypes());
			State = EState::ExpectMapRoot;
			return DcOk();
		}
		else if (Ret == ESQLitePreparedStatementStepResult::Done)
		{
			State = EState::ExpectArrayEnd;
			return DcOk();
		}
		else if (Ret == ESQLitePreparedStatementStepResult::Busy)
		{
			return DC_FAIL(DcDExtra, SqliteBusy);
		}
		else
		{
			return DC_FAIL(DcDExtra, SqliteLastError)
				<< Db->GetLastError();
		}
	}

	FDcResult PeekRead(EDcDataEntry* OutPtr) override
	{
		switch (State)
		{
		case EState::ExpectArrayRoot: return ReadOutOk(OutPtr, EDcDataEntry::ArrayRoot);
		case EState::ExpectArrayEnd: return ReadOutOk(OutPtr, EDcDataEntry::ArrayEnd);
		case EState::ExpectMapRoot: return ReadOutOk(OutPtr, EDcDataEntry::MapRoot);
		case EState::ExpectMapEnd: return ReadOutOk(OutPtr, EDcDataEntry::MapEnd);
		case EState::ExpectKey:  return ReadOutOk(OutPtr, EDcDataEntry::String);
		case EState::Ended:  return ReadOutOk(OutPtr, EDcDataEntry::Ended);
		case EState::ExpectValue: return ReadOutOk(OutPtr, SqliteColumnTypeToDataEntry(Types[ColIx]));
		}

		return DcOk();
	}

	FDcResult ReadArrayRoot() override
	{
		if (State != EState::ExpectArrayRoot)
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;

		return Step();
	}

	FDcResult ReadArrayEnd() override
	{
		if (State != EState::ExpectArrayEnd)
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;

		State = EState::Ended;
		return DcOk();
	}

	FDcResult ReadMapRoot() override
	{
		if (State != EState::ExpectMapRoot)
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;

		State = Names.Num() == 0
			? EState::ExpectMapEnd
			: EState::ExpectKey;

		return DcOk();
	}

	FDcResult ReadMapEnd() override
	{
		if (State != EState::ExpectMapEnd)
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;

		ColIx = 0;
		return Step();
	}

	template<typename TScalar>
	FDcResult ReadColValue(TScalar* OutPtr)
	{
		if (State != EState::ExpectValue)
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;

		if (!Stmt->GetColumnValueByIndex(ColIx, *OutPtr))
			return DC_FAIL(DcDExtra, SqliteLastError) << Db->GetLastError();

		++ColIx;
		State = ColIx == Names.Num()
			? EState::ExpectMapEnd
			: EState::ExpectKey;

		return DcOk();
	}

	FDcResult ReadName(FName* OutPtr) override
	{
		if (State == EState::ExpectKey)
		{
			State = EState::ExpectValue;
			FString& Str = Names[ColIx];
			if (Str.Len() >= NAME_SIZE)
				return DC_FAIL(DcDReadWrite, FNameOverSize);

			return ReadOutOk(OutPtr, FName(Str));
		}
		else if (State == EState::ExpectValue)
		{
			FString Str;
			DC_TRY(ReadColValue(&Str));

			if (Str.Len() >= NAME_SIZE)
				return DC_FAIL(DcDReadWrite, FNameOverSize);
			return ReadOutOk(OutPtr, FName(Str));
		}
		else
		{
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;
		}
	}

	FDcResult ReadString(FString* OutPtr) override
	{
		if (State == EState::ExpectKey)
		{
			State = EState::ExpectValue;
			return ReadOutOk(OutPtr, Names[ColIx]);
		}
		else if (State == EState::ExpectValue)
		{
			return ReadColValue(OutPtr);
		}
		else
		{
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;
		}
	}

	FDcResult ReadText(FText* OutPtr) override
	{
		FString Str;
		DC_TRY(ReadColValue(&Str));

		return ReadOutOk(OutPtr, FText::FromString(MoveTemp(Str)));
	}

	FDcResult ReadNil() override
	{
		if (State != EState::ExpectValue)
			return DC_FAIL(DcDReadWrite, InvalidStateNoExpect) << State;

		++ColIx;
		State = ColIx == Names.Num()
			? EState::ExpectMapEnd
			: EState::ExpectKey;

		return DcOk();
	}

	FDcResult ReadInt8(int8* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadInt16(int16* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadInt32(int32* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadInt64(int64* OutPtr) override { return ReadColValue(OutPtr); }

	FDcResult ReadUInt8(uint8* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadUInt16(uint16* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadUInt32(uint32* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadUInt64(uint64* OutPtr) override { return ReadColValue(OutPtr); }

	FDcResult ReadFloat(float* OutPtr) override { return ReadColValue(OutPtr); }
	FDcResult ReadDouble(double* OutPtr) override { return ReadColValue(OutPtr); }
	
	static FName ClassId() { return FName(TEXT("SqliteReader")); }
	FName GetId() override { return ClassId(); }
};

} // namespace SqliteDetails


FDcResult LoadStructArrayFromSQLite(FSQLiteDatabase* Db, const TCHAR* Query, FDcPropertyDatum Datum)
{
	using namespace SqliteDetails;

	FSQLitePreparedStatement Stmt = Db->PrepareStatement(Query, ESQLitePreparedStatementFlags::None);
	if (!Stmt.Execute())
		return DC_FAIL(DcDExtra, SqliteLastError)
			<< Db->GetLastError();

	LazyInitializeDeserializer();

	FSqliteReader Reader(Db, &Stmt);
	FDcPropertyWriter Writer(Datum);

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer.GetValue();
	Ctx.Properties.Add(Datum.Property);
	DC_TRY(Ctx.Prepare());
	DC_TRY(Deserializer->Deserialize(Ctx));

	return DcOk();
}

} // namespace DcExtra

DC_TEST("DataConfig.Extra.Sqlite")
{
	using namespace DcExtra;

	bool bSuccess = true;
	FSQLiteDatabase TestDb;
	bSuccess &= TestDb.Open(TEXT(":memory:"), ESQLiteDatabaseOpenMode::ReadWriteCreate);
	ON_SCOPE_EXIT { TestDb.Close(); };

	//	data fixture
	FString Statement = TEXT("CREATE TABLE users (id INTEGER NOT NULL,name TEXT, title TEXT)");
	bSuccess &= TestDb.Execute(*Statement);
	Statement = TEXT("INSERT INTO users (id, name, title) VALUES (1, 'John', 'Manager')");
	bSuccess &= TestDb.Execute(*Statement);
	Statement = TEXT("INSERT INTO users (id, name, title) VALUES (2, 'Mark', 'Engineer')");
	bSuccess &= TestDb.Execute(*Statement);
	Statement = TEXT("INSERT INTO users (id, name, title) VALUES (3, 'Bob', 'Engineer')");
	bSuccess &= TestDb.Execute(*Statement);
	Statement = TEXT("INSERT INTO users (id, name, title) VALUES (4, 'Mike', 'QA')");

	//	execute query and deserialize
	TArray<FDcExtraTestUser> Arr;
	UTEST_OK("Extra Sqlite", DcExtra::LoadStructArrayFromSQLite(
		&TestDb,
		TEXT("SELECT * FROM users WHERE title == 'Engineer' ORDER BY id"),
		Arr
		));

	//	equivalent fixture
	FString Fixture = TEXT(R"(
		[
			{
				"Id" : 2,
				"Name" : "Mark",
				"Title" : "Engineer"
			},
			{
				"Id" : 3,
				"Name" : "Bob",
				"Title" : "Engineer"
			}
		]
	)");

	FDcJsonWriter Writer;
	{
		using namespace DcPropertyUtils;
		auto ArrayProp = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::Struct(FDcExtraTestUser::StaticStruct())
		).LinkOnScope();

		UTEST_OK("Extra Sqlite", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(ArrayProp.Get(), &Arr)));
		Writer.Sb << TCHAR('\n');
	}

	UTEST_EQUAL("Extra Sqlite", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Fixture));
	UTEST_TRUE("Extra Sqlite", bSuccess);

	return true;
}
