#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mutex>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

unsigned long x {0};
std::mutex m;
void BM_mutex(benchmark::State& state) {
    for (auto _ : state) {
        REPEAT(
            {
                std::lock_guard<std::mutex> g(m);
                benchmark::DoNotOptimize(++x);
            }
        );
    }
    state.SetItemsProcessed(32*32*state.iterations());
}

void BM_mutex0(benchmark::State& state) {
    unsigned long x {0};
    std::mutex m;
    for (auto _ : state) {
        REPEAT(
            {
                std::lock_guard<std::mutex> g(m);
                benchmark::DoNotOptimize(++x);
            }
        );
    }
    state.SetItemsProcessed(32*32*state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);
#define ARG \
    ->ThreadRange(1, numcpu) \
    ->UseRealTime()

BENCHMARK(BM_mutex) ARG;
BENCHMARK(BM_mutex0) ARG;

BENCHMARK_MAIN();
