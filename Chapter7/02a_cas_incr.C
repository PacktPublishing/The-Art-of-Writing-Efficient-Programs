#include <optional>
#include <stack>
#include <mutex>
#include <unistd.h>

#include "spinlock.h"

#include "benchmark/benchmark.h"

class count {
    std::atomic<int> n_ = 0;
    public:
    int dec() {
        int n = n_.load(std::memory_order_relaxed);
        if (n == 0) return -1;
        int i = 0;
        while (!n_.compare_exchange_weak(n, n - 1, std::memory_order_release, std::memory_order_relaxed)) { nanosleep(i); };
        return n - 1;
    }
    int inc(int maxn) {
        int n = n_.load(std::memory_order_relaxed);
        if (n == maxn) return -1;
        int i = 0;
        while (!n_.compare_exchange_weak(n, n + 1, std::memory_order_release, std::memory_order_relaxed)) { nanosleep(i); };
        return n + 1;
    }
};

std::atomic<int> n0;

void BM_inc_dec0(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(n0.fetch_add(1, std::memory_order_release));
    benchmark::DoNotOptimize(n0.fetch_sub(1, std::memory_order_release));
  }
  state.SetItemsProcessed(state.iterations());
}
count n;

void BM_inc_dec(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(n.inc(100));
    benchmark::DoNotOptimize(n.dec());
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->Arg(1) \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_inc_dec0) ARGS;

BENCHMARK(BM_inc_dec) ARGS;

BENCHMARK_MAIN();
