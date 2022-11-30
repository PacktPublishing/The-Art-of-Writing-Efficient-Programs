#include <unistd.h>
#include <atomic>

#include "benchmark/benchmark.h"

class Ptrlock {
  public:
  Ptrlock(std::atomic<unsigned long*>& p) : p_(p), p_save_(NULL) {}
  unsigned long* lock() {
    static const timespec ns = { 0, 1 };
    unsigned long* p = nullptr;
    for (int i = 0; !p_.load(std::memory_order_relaxed) || !(p = p_.exchange(NULL, std::memory_order_acquire)); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
    return p_save_ = p;
  }
  void unlock() { p_.store(p_save_, std::memory_order_release); }
  private:
  std::atomic<unsigned long*>& p_;
  unsigned long* p_save_;
};

std::atomic<unsigned long*> p(new unsigned long);

void BM_lock(benchmark::State& state) {
  if (state.thread_index() == 0) *p.load() = 0;
  Ptrlock L(p);
  for (auto _ : state) {
    unsigned long* pl = L.lock();
    benchmark::DoNotOptimize(++*pl);
    L.unlock();
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_lock) ARGS;

BENCHMARK_MAIN();
