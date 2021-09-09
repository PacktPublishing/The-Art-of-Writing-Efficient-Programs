#include <atomic>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

std::atomic<unsigned long>* p(new std::atomic<unsigned long>);

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) *p = 0;
  constexpr size_t N = 1000000;
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) 
        benchmark::DoNotOptimize(p->fetch_add(1, std::memory_order_relaxed));
  }
  state.SetItemsProcessed(state.iterations()*N);
}

BENCHMARK(BM_lock) ARGS(1);
BENCHMARK(BM_lock) ARGS(2);
BENCHMARK(BM_lock) ARGS(4);

BENCHMARK_MAIN();
