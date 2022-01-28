#pragma once

#include "CoreMinimal.h"

struct FDcBenchRunResult
{
	TArray<uint64> Ticks;
	bool bAllOk;
};

DATACONFIGEXTRA_API FDcBenchRunResult DcBenchRun(int Iterations, TFunctionRef<void()> Body);

struct FDcBenchStat
{
	float MeanMs;
	float MedianMs;
	float Deviation;

	bool bAllOk;
};

DATACONFIGEXTRA_API FDcBenchStat DcBenchStats(TFunctionRef<bool()> Body);

DATACONFIGEXTRA_API FString DcFormatBenchStats(FString Prefix, double BytesCount, FDcBenchStat Stat);

DATACONFIGEXTRA_API FString DcBuildConfigurationString();

