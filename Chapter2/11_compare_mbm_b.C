// Microbenchmarks for string comparison using Google benchmark
#include <cstdlib>
#include <cstring>
#include <memory>

#include "benchmark/benchmark.h"

bool compare_int(const char* s1, const char* s2) {
    char c1, c2;
    for (int i1 = 0, i2 = 0; ; ++i1, ++i2) {
        c1 = s1[i1]; c2 = s2[i2];
        if (c1 != c2) return c1 > c2;
    }
}

