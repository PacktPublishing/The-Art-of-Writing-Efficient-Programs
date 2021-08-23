#include <unistd.h>
#include <atomic>
#include <mutex>

#include "benchmark/benchmark.h"

unsigned long* p(new unsigned long);
unsigned long& x = *p;

class Spinlock {
  public:
  Spinlock() : flag_(0) {}
  void lock() {
    static const timespec ns = { 0, 1 };
    for (int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
  }
  void unlock() { flag_.store(0, std::memory_order_release); }
  private:
  std::atomic<unsigned int> flag_;
};

Spinlock S;

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  for (auto _ : state) {
    std::lock_guard<Spinlock> L(S);
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
