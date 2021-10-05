# Changes

All notable changes to this project will be documented in this file.

## 1.1.1 - 2021.10.6

- Support for UE 4.27
- Support for UE 5

## 1.1.0 - 2021-4-24

- Integrate [nst/JSONTestSuite](https://github.com/nst/JSONTestSuite). Now `DcJSONParser` pass most of them. Skipped tests are also documented.
- `FDcAnsiJsonReader` now detect and convert non ASCII UTF8 characters. Previously these characters are dropped.
- Headless test runner pass along parameters to tests, for example
  `DataConfigHeadless-Win64-Debug.exe Parsing -- n_array_just_minus`