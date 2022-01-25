# celonis-regex-vectorization

# Benchmark

| size/algo   | std::regex  | parabix-ccp  | parabix-llvm |
| :---        |    :----:   |   :----:     |        :---: |
| 10MB        | 0.22        | 0.12         | <span style="color:red">0.016</span>        |
| 100MB       | 2.2         | 1.2          | <span style="color:red">0.12</span>         |
| 500MB       | 11          | 6            | <span style="color:red">0.6</span>          |
| 1GB         | 23          | 13           | <span style="color:red">1.2</span>          |
| 1.2GB       | 25          | 15           | <span style="color:red">1.4</span>          |

*NOTE: Time to read input data from a file is excluded from the elapsed times.*
