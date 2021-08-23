#include <unistd.h>
#include <string.h>
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

template <typename T>
class ts_unique_ptr {
    using ptr_t = std::shared_ptr<T>;
    public:
    ts_unique_ptr() = default;
    explicit ts_unique_ptr(T* p) : p_(p) {}
    ts_unique_ptr(const ts_unique_ptr&) = delete;
    ts_unique_ptr& operator=(const ts_unique_ptr&) = delete;
    ~ts_unique_ptr() = default;
    void publish(ptr_t p) noexcept { std::atomic_store_explicit(&p_, p, std::memory_order_release); }
    const T* get() const noexcept { return std::atomic_load_explicit(&p_, std::memory_order_acquire).get(); }
    const T& operator*() const noexcept { return *this->get(); }
    ts_unique_ptr& operator=(ptr_t p) noexcept { this->publish(p); return *this; }
    private:
    ptr_t p_;
};

ts_unique_ptr<A> p(new A(42));

void BM_ptr_deref(benchmark::State& state) {
  volatile A x;
  for (auto _ : state) {
    REPEAT(benchmark::DoNotOptimize(x = *p););
  }
  state.SetItemsProcessed(32*state.iterations());
}

std::shared_ptr<A> q(new A(7));

void BM_ptr_assign(benchmark::State& state) {
  for (auto _ : state) {
    REPEAT(benchmark::DoNotOptimize(p = q););
  }
  state.SetItemsProcessed(32*state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_ptr_deref) ARGS;
BENCHMARK(BM_ptr_assign) ARGS;

BENCHMARK_MAIN();
