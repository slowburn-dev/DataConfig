# Benchmark

We've integrated two benchmarks fixtures from [JSON for modern C++][1] project. In the benchmark we deserialize JSON into C++ structs, then serialize them back to JSON. Then we convert JSON to MsgPack and do identical process.

* [canada.json]({{RepoRoot}}DataConfig/Tests/LargeFixtures/canada.json)
* [corpus.ndjson]({{RepoRoot}}DataConfig/Tests/LargeFixtures/corpus.ndjson)

See [here for instructions](./Automation.md#running-the-benchmarks) to build and run the benchmark.

Here're the results:

* [DcBenchmarkFixture1.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Benchmark/DcBenchmarkFixture1.cpp)

On a Intel(R) Core(TM) i7-7700K CPU @ 4.20GHz:

```
Corpus Json Deserialize: [Shipping] Bandwidth: 77.816(MB/s), Mean: 70.891(ms), Median:70.662(ms), Deviation:0.861
Corpus Json Serialize: [Shipping] Bandwidth: 97.532(MB/s), Mean: 56.560(ms), Median:55.786(ms), Deviation:1.930
Corpus MsgPack Deserialize: [Shipping] Bandwidth: 89.347(MB/s), Mean: 58.498(ms), Median:58.339(ms), Deviation:0.573
Corpus MsgPack Serialize: [Shipping] Bandwidth: 79.530(MB/s), Mean: 65.719(ms), Median:65.457(ms), Deviation:0.867

Canada Json Deserialize: [Shipping] Bandwidth: 59.257(MB/s), Mean: 36.228(ms), Median:36.029(ms), Deviation:0.490
Canada Json Serialize: [Shipping] Bandwidth: 47.083(MB/s), Mean: 45.596(ms), Median:44.961(ms), Deviation:1.563
Canada MsgPack Serialize: [Shipping] Bandwidth: 101.112(MB/s), Mean: 5.778(ms), Median:5.764(ms), Deviation:0.060
Canada MsgPack Deserialize: [Shipping] Bandwidth: 89.467(MB/s), Mean: 6.530(ms), Median:6.513(ms), Deviation:0.094
```

On a AMD Ryzen 9 5950X 16-Core Processor 3.40 GHz:

```
Corpus Json Deserialize: [Shipping] Bandwidth: 98.890(MB/s), Mean: 55.784(ms), Median:55.486(ms), Deviation:0.856
Corpus Json Serialize: [Shipping] Bandwidth: 107.389(MB/s), Mean: 51.369(ms), Median:51.046(ms), Deviation:1.531
Corpus MsgPack Deserialize: [Shipping] Bandwidth: 112.146(MB/s), Mean: 46.606(ms), Median:46.521(ms), Deviation:0.213
Corpus MsgPack Serialize: [Shipping] Bandwidth: 98.295(MB/s), Mean: 53.173(ms), Median:52.951(ms), Deviation:1.065

Canada Json Deserialize: [Shipping] Bandwidth: 72.858(MB/s), Mean: 29.465(ms), Median:29.238(ms), Deviation:0.802
Canada Json Serialize: [Shipping] Bandwidth: 57.586(MB/s), Mean: 37.279(ms), Median:37.363(ms), Deviation:0.640
Canada MsgPack Serialize: [Shipping] Bandwidth: 126.463(MB/s), Mean: 4.620(ms), Median:4.548(ms), Deviation:0.233
Canada MsgPack Deserialize: [Shipping] Bandwidth: 103.348(MB/s), Mean: 5.653(ms), Median:5.649(ms), Deviation:0.020
```

Some insights on the results:

- Benchmark in `Shipping` build configuration, otherwise it doesn't make much sense.

- Recall that [runtime performance isn't our top priority](../Design.md#manifesto). We opted for a classic inheritance based API for `FDcReader/FDcWriter` 
  which means that each read/write step result in a virtual dispatch. This by design would result in mediocre performance metrics.
  The bandwidth should be in the range of `10~100(MB/s)` on common PC setup, no matter how simple the format is.

- MsgPack and JSON has similar bandwidth numbers in the benchmark. However MsgPack has far more tight layout when dealing with 
  numeric data. Note in the `Canada` fixture MsgPack only takes around 10ms, as this fixture is mostly float number coordinates.


[1]:https://json.nlohmann.me "JSON for Modern C++"
