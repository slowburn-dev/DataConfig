# Benchmark

We've integrated two benchmarks fixtures from [JSON for modern C++][1] project. In the benchmark we deserialize JSON into C++ structs, then serialize them back to JSON. Then we convert JSON to MsgPack and do identical process.

Here're the results:

```
Corpus Json Deserialize: [Shipping] Bandwidth: 64.109(MB/s), Mean: 86.047(ms), Median:85.737(ms), Deviation:1.103
Corpus Json Serialize: [Shipping] Bandwidth: 74.531(MB/s), Mean: 74.015(ms), Median:73.505(ms), Deviation:2.318
Corpus MsgPack Deserialize: [Shipping] Bandwidth: 75.873(MB/s), Mean: 68.887(ms), Median:68.755(ms), Deviation:0.457
Corpus MsgPack Serialize: [Shipping] Bandwidth: 62.885(MB/s), Mean: 83.115(ms), Median:83.020(ms), Deviation:0.434

Canada Json Deserialize: [Shipping] Bandwidth: 56.027(MB/s), Mean: 38.317(ms), Median:38.175(ms), Deviation:0.513
Canada Json Serialize: [Shipping] Bandwidth: 43.415(MB/s), Mean: 49.448(ms), Median:49.129(ms), Deviation:1.414
Canada MsgPack Serialize: [Shipping] Bandwidth: 67.366(MB/s), Mean: 8.672(ms), Median:8.669(ms), Deviation:0.021
Canada MsgPack Deserialize: [Shipping] Bandwidth: 52.600(MB/s), Mean: 11.107(ms), Median:11.064(ms), Deviation:0.173
```

Some insights on the results:

- Benchmark in `Shipping` build configuration, otherwise it doesn't make much sense.

- Recall that [runtime performance isn't our top priority](../Design.md#manifesto). We opted for a classic inheritance based API for `FDcReader/FDcWriter` 
  which means that each read/write step result in a virtual dispatch. This by design would result in mediocre performance metrics.
  The bandwidth should be in the range of `10~100(MB/s)` on common PC setup, no matter how simple the format is.

- MsgPack and JSON has similar bandwidth numbers in the benchmark. However MsgPack has far more tight layout when dealing with 
  numeric data. Note in the `Canada` fixture MsgPack only takes around 10ms, as this fixture is mostly float number coordinates.


[1]:https://json.nlohmann.me "JSON for Modern C++"
