# celonis-regex-vectorization

# Benchmark

| size/algo   | std::regex  | parabix-ccp  | parabix-llvm |
| :---        |    :----:   |   :----:     |        :---: |
| 10MB        | 0.23        | 0.13         | <span style="color:red">0.021</span>        |
| 100MB       | 2.3         | 1.3          | <span style="color:red">0.16</span>         |
| 500MB       | 12          | 7            | <span style="color:red">0.81</span>         |
| 1GB         | 25          | 14           | <span style="color:red">1.6</span>          |

*NOTE: Time to read input data from a file is excluded from the elapsed times.*
