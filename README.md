# Parabix LLVM

This repository contains the implementation for the thesis "[High-Performance Regular Expression Matching with Parabix and LLVM](https://www.semanticscholar.org/paper/High-Performance-Regular-Expression-Matching-with-Denis/d4cb59d780f93f308c8591bda49cb766db24c520)" which can also be found [here](https://core.ac.uk/download/pdf/56378923.pdf).

This project was done a part of [TUM Database Implementation](https://db.in.tum.de/teaching/ws2122/imlab/?lang=en) practical course.

# Implementation

This repository contains both iterative and LLVM codegen approaches for Parabix, they are located at [parabix_cpp](src/parabix/parabix.cc#L38) and [parabix_llvm](src/parabix/parabix.cc#L120) relatively.

You may also want to check the Parabix compiler ([parabix_compiler.cc](src/codegen/parabix_compiler.cc)) that generates a code by LLVM IRBuilder API.

# Presentation

You can find the PDF document [here](presentation/parabix-llvm.pdf) used during the presentation.

# Benchmark

Relative files are [generator](generator/main.cc) and [benchmark](tools/benchmark.cc).

| size/algo   | std::regex  | parabix-ccp  | parabix-llvm |
| :---        |    :----:   |   :----:     |        :---: |
| 10MB        | 0.22        | 0.12         | <span style="color:red">0.016</span>        |
| 100MB       | 2.2         | 1.2          | <span style="color:red">0.12</span>         |
| 500MB       | 11          | 6            | <span style="color:red">0.6</span>          |
| 1GB         | 23          | 13           | <span style="color:red">1.2</span>          |
| 1.2GB       | 25          | 15           | <span style="color:red">1.4</span>          |

*NOTE: Time to read input data from a file is excluded from the elapsed times. The pattern is <b>a[0-9]\*z</b>.*

# Wanna try?
```sh
mkdir build
cd build
cmake ../
# generate input file
ninja generator
./generator 1000 ../1gb.txt
# run benchmark
ninja benchmark
./benchmark
# run vgrep
ninja vgrep_llvm
./vgrep_llvm ../1gb.txt "a[0-9]*z"
```
