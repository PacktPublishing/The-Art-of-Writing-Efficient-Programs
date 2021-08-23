// Microbenchmarks for string comparison using Google benchmark (11c without temp variables)
#include <cstdlib>
#include <cstring>
#include <memory>

#include "benchmark/benchmark.h"

using std::unique_ptr;

bool compare_int_len_notmp(const char* s1, const char* s2, int l) {
    for (int i1 = 0, i2 = 0; i1 < l; ++i1, ++i2) {
        if (s1[i1] != s2[i2]) return s1[i1] > s2[i2];
    }
    return false;
}

void BM_loop_int_len_notmp(benchmark::State& state) {
    const unsigned int N = state.range(0);
    unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare_int_len_notmp(s1, s2, N));
    }
    state.SetItemsProcessed(N*state.iterations());
}

#define ARGS \
    ->Arg(1<<20)

BENCHMARK(BM_loop_int_len_notmp) ARGS;
BENCHMARK_MAIN();
