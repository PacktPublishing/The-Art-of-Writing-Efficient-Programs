#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <atomic>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

std::atomic<unsigned long> x1(0);
void BM_local(benchmark::State& state) {
    unsigned long x = 0;
    const size_t N = state.range(0);
    for (auto _ : state) {
        for (size_t i = 0; i < N; i += 32) {
            REPEAT(benchmark::DoNotOptimize(++x););
        }
        x1 += x;
    }
    state.SetItemsProcessed(N*state.iterations());
}

std::atomic<unsigned long> x2(0);
unsigned long x2a[1024];
void BM_false_shared(benchmark::State& state) {
    unsigned long& x = x2a[state.thread_index];
    const size_t N = state.range(0);
    for (auto _ : state) {
        for (size_t i = 0; i < N; i += 32) {
            REPEAT(benchmark::DoNotOptimize(++x););
        }
        x2 += x;
    }
    state.SetItemsProcessed(N*state.iterations());
}

std::atomic<unsigned long> x3(0);
unsigned long x3a[16*1024];
void BM_not_shared(benchmark::State& state) {
    unsigned long& x = x3a[16*state.thread_index];
    const size_t N = state.range(0);
    for (auto _ : state) {
        for (size_t i = 0; i < N; i += 32) {
            REPEAT(benchmark::DoNotOptimize(++x););
        }
        x3 += x;
    }
    state.SetItemsProcessed(N*state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);
#define ARG \
    ->Arg(1024) \
    ->ThreadRange(1, numcpu) \
    ->UseRealTime()

//BENCHMARK(BM_local) ARG;
BENCHMARK(BM_false_shared) ARG;
BENCHMARK(BM_not_shared) ARG;

BENCHMARK_MAIN();
