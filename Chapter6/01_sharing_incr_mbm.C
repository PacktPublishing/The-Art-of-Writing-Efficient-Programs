#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <iostream>

#include "benchmark/benchmark.h"

using namespace std;

#define REPEAT2(x) {x} {x}
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT64(x) REPEAT32(x) REPEAT32(x)
#define REPEAT(x) REPEAT64(x)

std::atomic<unsigned long> xa(0);

void BM_atomic(benchmark::State& state) {
  std::atomic<unsigned long>& x = xa;
  for (auto _ : state) {
    REPEAT(benchmark::DoNotOptimize(++x););
  }
  state.SetItemsProcessed(32*state.iterations());
}

unsigned long x = 0;
std::mutex M;

void BM_mutex(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  for (auto _ : state) {
    std::lock_guard<std::mutex> L(M);
    benchmark::DoNotOptimize(++x);
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_cas(benchmark::State& state) {
  std::atomic<unsigned long>& x = xa;
  if (state.thread_index == 0) x = 0;
  for (auto _ : state) {
    unsigned long xl = x.load(std::memory_order_relaxed);
    while (!x.compare_exchange_strong(xl, xl + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {}
  }
  state.SetItemsProcessed(state.iterations());
}

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

void BM_spinlock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  for (auto _ : state) {
    std::lock_guard<Spinlock> L(S);
    benchmark::DoNotOptimize(++x);
  }
  state.SetItemsProcessed(state.iterations());
}

class Ptrlock {
  public:
  Ptrlock(std::atomic<unsigned long*>& p) : p_(p), p_save_(NULL) {}
  unsigned long* lock() {
    static const timespec ns = { 0, 1 };
    for (int i = 0; !p_.load(std::memory_order_relaxed) || !(p_save_ = p_.exchange(NULL, std::memory_order_acquire)); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
    return p_save_;
  }
  void unlock() { p_.store(p_save_, std::memory_order_release); }
  private:
  std::atomic<unsigned long*>& p_;
  unsigned long* p_save_;
};

std::atomic<unsigned long*> p(new unsigned long);

void BM_ptrlock(benchmark::State& state) {
  if (state.thread_index == 0) *p.load() = 0;
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

BENCHMARK(BM_atomic) ARGS;
BENCHMARK(BM_mutex) ARGS;
BENCHMARK(BM_cas) ARGS;
BENCHMARK(BM_spinlock) ARGS;
BENCHMARK(BM_ptrlock) ARGS;

BENCHMARK_MAIN();
