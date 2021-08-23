#include <unistd.h>
#include <atomic>
#include <mutex>

#include "benchmark/benchmark.h"

unsigned long x = 0;
std::mutex M;

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  for (auto _ : state) {
    std::lock_guard<std::mutex> L(M);
    benchmark::DoNotOptimize(++x);
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_lock) ARGS;

BENCHMARK_MAIN();
