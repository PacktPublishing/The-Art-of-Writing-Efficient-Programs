#include <stdlib.h>
#include <string.h>
#include <deque>

#include "benchmark/benchmark.h"

void BM_index(benchmark::State& state) {
    const unsigned int N = state.range(0);
    std::deque<unsigned long> d(N);
    for (size_t i = 0; i < N; ++i) {
        d[i] = rand();
    }
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            benchmark::DoNotOptimize(d[i]);
        }
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_iter(benchmark::State& state) {
    const unsigned int N = state.range(0);
    std::deque<unsigned long> d(N);
    for (size_t i = 0; i < N; ++i) {
        d[i] = rand();
    }
    for (auto _ : state) {
        for (auto it = d.cbegin(), it0 = d.cend(); it != it0; ++it) {
            benchmark::DoNotOptimize(*it);
        }
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

#define ARGS \
    ->Arg(1<<22)

BENCHMARK(BM_index) ARGS;
BENCHMARK(BM_iter) ARGS;

BENCHMARK_MAIN();

