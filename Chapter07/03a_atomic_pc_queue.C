#include <optional>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <unistd.h>
#include <string.h>

#include "spinlock.h"

#include "benchmark/benchmark.h"

struct LS {
    long x[1024];
    LS(char i) { for (size_t k = 0; k < 1024; ++k) x[k] = i; }
};

template <typename T> class pc_queue {
  public:
  explicit pc_queue(size_t capacity) : 
    capacity_(capacity), data_(static_cast<T*>(::malloc(sizeof(T)*capacity_))) {}
  ~pc_queue() { ::free(data_); }
  bool push(const T& v) {
    if (size_.load(std::memory_order_relaxed) == capacity_) return false;
    new (data_ + (back_ % capacity_)) T(v);
    ++back_;
    size_.fetch_add(1, std::memory_order_release);
    return true;
  }
  std::optional<T> pop() {
    if (size_.load(std::memory_order_acquire) == 0) {
      return std::optional<T>(std::nullopt);
    } else {
      std::optional<T> res(std::move(data_[front_ % capacity_]));
      data_[front_ % capacity_].~T();
      ++front_;
      size_.fetch_sub(1, std::memory_order_relaxed);
      return res;
    }
  }

  void reset() { size_ = front_ = back_ = 0; }
  private:
  const size_t capacity_;
  T* const data_;
  size_t front_ = 0;
  size_t back_ = 0;
  std::atomic<size_t> size_;
};

pc_queue<int> q(1UL << 15);
pc_queue<LS> lq(1UL << 15);

void BM_queue_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) q.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) q.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(q.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_lqueue_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) lq.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) lq.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(lq.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->Arg(1) \
  ->Threads(2) \
  ->UseRealTime()

BENCHMARK(BM_queue_prod_cons) ARGS;
BENCHMARK(BM_lqueue_prod_cons) ARGS;

BENCHMARK_MAIN();
