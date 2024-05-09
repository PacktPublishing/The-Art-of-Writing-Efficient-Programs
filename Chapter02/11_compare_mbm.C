// Microbenchmarks for string comparison using Google benchmark
// Build as follows:
// $CXX 11_compare_mbm.C 11_compare_mbm_a.C 11_compare_mbm_b.C 11_compare_mbm_c.C -I$GBENCH_DIR/include -g -O3 -I. --std=c++17 $GBENCH_DIR/lib/libbenchmark.a -lpthread -latomic -lrt -lm -o 01a_substring_sort
#include <cstdlib>
#include <cstring>
#include <memory>

#include "benchmark/benchmark.h"

using std::unique_ptr;

extern bool compare_int(const char* s1, const char* s2);

extern bool compare_uint(const char* s1, const char* s2);

extern bool compare_uint_l(const char* s1, const char* s2, unsigned int l);

void BM_loop_int(benchmark::State& state) {
    const unsigned int N = state.range(0);
    unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare_int(s1, s2));
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_loop_uint(benchmark::State& state) {
    const unsigned int N = state.range(0);
    unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare_uint(s1, s2));
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_loop_uint_l(benchmark::State& state) {
    const unsigned int N = state.range(0);
    unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare_uint_l(s1, s2, 2*N));
    }
    state.SetItemsProcessed(N*state.iterations());
}

#define ARGS \
    ->Arg(1<<20)

BENCHMARK(BM_loop_int) ARGS;
BENCHMARK(BM_loop_uint) ARGS;
BENCHMARK(BM_loop_uint_l) ARGS;

BENCHMARK_MAIN();

