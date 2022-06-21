# Deserialize From SQLite Query

Unreal Engine bundles SQLite3 in SQLiteCore plugin and bundled with minimal C++ abstractions. In this example we implemented `LoadStructArrayFromSQLite` to load a SQLite query into a `TArray` of structs:

* [DcSqlite.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/Misc/DcSqlite.h)
* [DcSqlite.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Misc/DcSqlite.cpp)

```c++
// DataConfigExtra/Public/DataConfig/Extra/Misc/DcSqlite.h
template<typename TStruct>
FDcResult LoadStructArrayFromSQLite(FSQLiteDatabase* Db, const TCHAR* Query, TArray<TStruct>& Arr)
// ...
```

With this method we can easily turn SQLite query results into structs that's easy to manipulate with:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Misc/DcSqlite.cpp
/// data fixture
FString Statement = TEXT("CREATE TABLE users (id INTEGER NOT NULL,name TEXT, title TEXT)");
bSuccess &= TestDb.Execute(*Statement);
Statement = TEXT("INSERT INTO users (id, name, title) VALUES (1, 'John', 'Manager')");
bSuccess &= TestDb.Execute(*Statement);
Statement = TEXT("INSERT INTO users (id, name, title) VALUES (2, 'Mark', 'Engineer')");
bSuccess &= TestDb.Execute(*Statement);
Statement = TEXT("INSERT INTO users (id, name, title) VALUES (3, 'Bob', 'Engineer')");
bSuccess &= TestDb.Execute(*Statement);
Statement = TEXT("INSERT INTO users (id, name, title) VALUES (4, 'Mike', 'QA')");

//  execute query and deserialize
TArray<FDcExtraTestUser> Arr;
UTEST_OK("Extra Sqlite", DcExtra::LoadStructArrayFromSQLite(
    &TestDb,
    TEXT("SELECT * FROM users WHERE title == 'Engineer' ORDER BY id"),
    Arr
    ));

//  equivalent fixture
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
```

For this to work we'll need to implement `FSqliteReader` which implements `FDcReader` API so it can be consumed by deserializer. The cool thing is that `FSqliteReader` works very well with SQLite's step API and can wrap SQLite error reporting into diagnostics that DataConfig can report.

