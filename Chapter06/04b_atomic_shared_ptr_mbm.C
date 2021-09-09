#include <unistd.h>
#include <atomic>
#include <memory>

#include "benchmark/benchmark.h"

#define REPEAT2(x) {x} {x}
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT64(x) REPEAT32(x) REPEAT32(x)
#define REPEAT(x) REPEAT64(x)

using namespace std;

struct A {
  int i;
  A(int i = 0) : i(i) {}
  A& operator=(const A& rhs) { i = rhs.i; return *this; }
  volatile A& operator=(const A& rhs) volatile { i = rhs.i; return *this; }
};

std::shared_ptr<A> p(std::make_shared<A>(42));

void BM_ptr_deref(benchmark::State& state) {
  volatile A x;
  for (auto _ : state) {
    benchmark::DoNotOptimize(x = *(std::atomic_load_explicit(&p, std::memory_order_relaxed)));
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_ptr_copy(benchmark::State& state) {
  for (auto _ : state) {
    volatile std::shared_ptr<A> q(std::atomic_load_explicit(&p, std::memory_order_relaxed));
  }
  state.SetItemsProcessed(state.iterations());
}

std::shared_ptr<A> q(std::make_shared<A>(7));

void BM_ptr_assign(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(q = std::atomic_load_explicit(&p, std::memory_order_relaxed));
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_ptr_assign1(benchmark::State& state) {
  for (auto _ : state) {
      std::atomic_store_explicit(&q, std::atomic_load_explicit(&p, std::memory_order_relaxed), std::memory_order_relaxed);
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_ptr_xassign(benchmark::State& state) {
  if (state.thread_index == 0) p = std::shared_ptr<A>(new A(42)), q = std::shared_ptr<A>(new A(7));
  if (state.thread_index & 1) {
    for (auto _ : state) {
      benchmark::DoNotOptimize(q = std::atomic_load_explicit(&p, std::memory_order_relaxed));
    }
  } else {
    for (auto _ : state) {
      benchmark::DoNotOptimize(p = std::atomic_load_explicit(&q, std::memory_order_relaxed));
    }
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_ptr_deref) ARGS;
BENCHMARK(BM_ptr_copy) ARGS;
BENCHMARK(BM_ptr_assign) ARGS;
BENCHMARK(BM_ptr_assign1) ARGS;
BENCHMARK(BM_ptr_xassign) ARGS;

BENCHMARK_MAIN();
