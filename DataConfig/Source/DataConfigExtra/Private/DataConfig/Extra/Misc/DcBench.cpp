#include "DataConfig/Extra/Misc/DcBench.h"
#include "HAL/PlatformTime.h"

FDcBenchRunResult DcBenchRun(int Iterations, TFunctionRef<bool()> Body)
{
	FDcBenchRunResult Result;
	Result.Ticks.AddUninitialized(Iterations + 1);
	Result.bAllOk = true;

	int Ix = 0;
	for (Ix = 0; Ix < Iterations; Ix++)
	{
		Result.Ticks[Ix] = FPlatformTime::Cycles64();
		Result.bAllOk = Body();
		if (!Result.bAllOk)
			break;
	}
	Result.Ticks[Ix] = FPlatformTime::Cycles64();

	return Result;
}

constexpr static int BENCH_ITER_WARM_UP = 5;
constexpr static int BENCH_ITER_RUN = 30;

FDcBenchStat DcBenchStats(TFunctionRef<bool()> Body)
{
	FDcBenchStat Stat {0};
	FDcBenchRunResult Warm = DcBenchRun(BENCH_ITER_WARM_UP, Body);
	if (!Warm.bAllOk)
	{
		Stat.bAllOk = false;
		return Stat;
	}

	FDcBenchRunResult Run = DcBenchRun(BENCH_ITER_RUN, Body);
	Stat.bAllOk = Run.bAllOk;
	if (Stat.bAllOk)
	{
		TArray<int64> Intervals;
		Intervals.AddUninitialized(BENCH_ITER_RUN);
		for (int Ix = 0; Ix < BENCH_ITER_RUN; Ix++)
			Intervals[Ix] = (int64)(Run.Ticks[Ix + 1] - Run.Ticks[Ix]);

		Intervals.Sort();

		int64 Total = 0;
		for (int Ix = 0; Ix < BENCH_ITER_RUN; Ix++)
			Total += Intervals[Ix];

		Stat.MeanMs = FPlatformTime::ToMilliseconds(Total / BENCH_ITER_RUN);
		Stat.MedianMs = FPlatformTime::ToMilliseconds(Intervals[BENCH_ITER_RUN / 2]);

		double DevAcc = 0;
		for (int Ix = 0; Ix < BENCH_ITER_RUN; Ix++)
			DevAcc += FMath::Square(FPlatformTime::ToMilliseconds64(Intervals[Ix]) - Stat.MeanMs);

		Stat.Deviation = FMath::Sqrt((float)(DevAcc / BENCH_ITER_RUN));
	}
	return Stat;
}

FString DcFormatBenchStats(FString Prefix, double BytesCount, FDcBenchStat Stat)
{
	if (Stat.bAllOk)
	{
		float MegaByte = ((float)BytesCount / (1024 * 1024));
		float Seconds = (Stat.MeanMs * 0.001f);
		float MegaBytesPerSecond = MegaByte / Seconds;

		return FString::Printf(
			TEXT("%s: [%s] Bandwidth: %.3f(MB/s), Mean: %.3f(ms), Median:%.3f(ms), Deviation:%.3f"),
			*Prefix,
			*DcBuildConfigurationString(),
			MegaBytesPerSecond, 
			Stat.MeanMs,
			Stat.MedianMs,
			Stat.Deviation
		);
	}
	else
	{
		return FString::Printf(
			TEXT("%s: [%s] runtime error, no benchmark stats"),
			*Prefix,
			*DcBuildConfigurationString()
		);
	}
}

FString DcBuildConfigurationString()
{
#if UE_BUILD_SHIPPING
	return TEXT("Shipping");
#elif UE_BUILD_DEVELOPMENT
	return TEXT("Development");
#elif UE_BUILD_DEBUG
	return TEXT("Debug");
#elif UE_BUILD_TEST
	return TEXT("Test");
#else
	return TEXT("<UNKNOWN>");
#endif
}
