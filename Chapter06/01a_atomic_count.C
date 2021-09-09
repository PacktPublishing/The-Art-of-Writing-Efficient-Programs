#include <unistd.h>
#include <atomic>

#include "benchmark/benchmark.h"

std::atomic<unsigned long>* p(new std::atomic<unsigned long>);

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) *p = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(p->fetch_add(1, std::memory_order_relaxed));
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_lock) ARGS;

BENCHMARK_MAIN();
