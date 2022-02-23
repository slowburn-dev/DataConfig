#pragma once

#include "CoreMinimal.h"
#include "DcBenchmarkFixture1.generated.h"

USTRUCT()
struct FDcVector2D
{
	GENERATED_BODY()

	UPROPERTY() float X;
	UPROPERTY() float Y;
};

///	Structs for deserialize `canada.json`
USTRUCT()
struct FDcCanadaCoords
{
	GENERATED_BODY()

	//	TArray<TArray<FDcVector2D>> isn't supported, need a wrapper struct
	UPROPERTY() TArray<FDcVector2D> data;
};

USTRUCT()
struct FDcCanadaGeometry
{
	GENERATED_BODY()

	UPROPERTY() FString type;
	UPROPERTY() TArray<FDcCanadaCoords> coordinates;
};

USTRUCT()
struct FDcCanadaFeature
{
	GENERATED_BODY()

	UPROPERTY() FString type;
	UPROPERTY() TMap<FString, FString> properties;
	UPROPERTY() FDcCanadaGeometry geometry;
};

USTRUCT()
struct FDcCanadaRoot
{
	GENERATED_BODY()

	UPROPERTY() FString type;
	UPROPERTY() TArray<FDcCanadaFeature> features;
};

///	Structs for deserialize `corpus.ndjson`


USTRUCT()
struct FDcCorpusAuthor
{
	GENERATED_BODY()
	
	UPROPERTY() FString name;
	UPROPERTY() TArray<FString> ids;
	UPROPERTY() FString structuredName;
};


USTRUCT()
struct FDcCorpusEntry
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString title;
	UPROPERTY() FString paperAbstract;
	UPROPERTY() TArray<FDcCorpusAuthor> authors;
	UPROPERTY() TArray<FString> inCitations;
	UPROPERTY() TArray<FString> outCitations;
	UPROPERTY() int year;
	UPROPERTY() FString s2Url;
	UPROPERTY() TArray<FString> sources;
	UPROPERTY() TArray<FString> pdfUrls;
	UPROPERTY() FString venue;
	UPROPERTY() FString journalName;
	UPROPERTY() FString journalVolume;
	UPROPERTY() FString journalPages;
	UPROPERTY() FString doi;
	UPROPERTY() FString doiUrl;
	UPROPERTY() FString pmid;
	UPROPERTY() TArray<FString> fieldsOfStudy;
	UPROPERTY() FString magId;
	UPROPERTY() FString s2PdfUrl;
	UPROPERTY() TArray<FString> entities;
};

USTRUCT()
struct FDcCorpusRoot
{
	GENERATED_BODY()
	
	UPROPERTY() TArray<FDcCorpusEntry> data;
};
