# HyperLogLog CPU Baseline for x86 + AVX2
Baseline implementation for evaluating HyperLogLog for a selection of hashes
on an x86 server CPU.

# Contributors
### ETH Systems Group
- Amit Kulkarni <amit.kulkarni@ieee.org>
- Kaan Kara <kaan.kara@outlook.com>
- Thomas Preußer <thomas.preusser@utexas.edu>

# Quick Start
### Collect Estimation Errors
```bash
> make error
```
The error data obtained for different hash functions is tabulated in `error.dat`.

### Run Performance Benchmarks
```bash
> make bench
```
The results for all runs are collected in `bench.log`.

# Sources
File | Contents
-----|-----
[hyperloglog.hpp](hyperloglog.hpp) | Header exposing the available choices for hashes as well as the sketch collection and estimation interface.
[hyperloglog.cpp](hyperloglog.cpp) | Implementation of the sketch infrastructure.
[hll_base.cpp](hll_base.cpp)       | Implementations of non-AVX sketch collectors for various hash functions.
[hll_avx.cpp](hll_avx.cpp)         | Sketch collector implementations leveraging AVX2 for the Murmur3 hash variants.
[err_vs_card.cpp](err_vs_card.cpp) | Application evaluating the HLL estimation quality for the specified hashes.
[hll_bench.cpp](hll_bench.cpp)     | Benchmark application reporting on HLL quality and performance of the specified run.
