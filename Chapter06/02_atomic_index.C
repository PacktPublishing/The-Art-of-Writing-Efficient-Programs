#include <unistd.h>
#include <atomic>

#include "benchmark/benchmark.h"

class AtomicCount {
    std::atomic<unsigned long> c_;
    public:
    unsigned long incr() noexcept { return 1 + c_.fetch_add(1, std::memory_order_relaxed); }
    unsigned long get() const noexcept { return c_.load(std::memory_order_relaxed); }
};

class AtomicIndex {
    std::atomic<unsigned long> c_;
    public:
    unsigned long incr() noexcept { return 1 + c_.fetch_add(1, std::memory_order_release); }
    unsigned long get() const noexcept { return c_.load(std::memory_order_acquire); }
};

AtomicCount* ac = new AtomicCount;
AtomicIndex* ai = new AtomicIndex;

void BM_count(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(ac->incr());
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_index(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(ai->incr());
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_count) ARGS;

BENCHMARK_MAIN();
