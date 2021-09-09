#include <optional>
#include <deque>
#include <mutex>
#include <unistd.h>

#include "spinlock.h"

#include "benchmark/benchmark.h"

template <typename T> void reset(T& s) { T().swap(s); }

template <typename T> class mt_stack
{
    std::deque<T> s_;
    mutable spinlock l_;
    int cap_ = 0;
    struct counts_t { 
        int pc_ = 0; // Producer count
        int cc_ = 0; // Consumer count
        bool equal(std::atomic<counts_t>& n) {
            if (pc_ == cc_) return true;
            *this = n.load(std::memory_order_relaxed);
            return false;
        }
    };
    mutable std::atomic<counts_t> n_;
    public:
    mt_stack(size_t n = 100000000) : s_(n), cap_(n) {}
    void push(const T& v) {
        // Reserve the slot for the new object by advancing the producer count.
        counts_t n = n_.load(std::memory_order_relaxed);
        if (n.pc_ == cap_) abort();
        int i = 0;
        while (!n.equal(n_) || !n_.compare_exchange_weak(n, {n.pc_ + 1, n.cc_}, std::memory_order_acquire, std::memory_order_relaxed)) {
            if (n.pc_ == cap_) abort();
            nanosleep(i);
        };
        // Producer count advanced, slot n.pc_ + 1 is ours.
        ++n.pc_;
        new (&s_[n.pc_]) T(v);

        // Advance the consumer count to match.
        if (!n_.compare_exchange_strong(n, {n.pc_, n.cc_ + 1}, std::memory_order_release, std::memory_order_relaxed)) abort();
    }
    std::optional<T> pop() {
        // Decrement the consumer count.
        counts_t n = n_.load(std::memory_order_relaxed);
        if (n.cc_ == 0) return std::optional<T>(std::nullopt);
        int i = 0;
        while (!n.equal(n_) || !n_.compare_exchange_weak(n, {n.pc_, n.cc_ - 1}, std::memory_order_acquire, std::memory_order_relaxed)) { 
            if (n.cc_ == 0) return std::optional<T>(std::nullopt);
            nanosleep(i);
        };
        // Consumer count decremented, slot n.cc_ - 1 is ours.
        --n.cc_;
        std::optional<T> res(std::move(s_[n.pc_]));
        s_[n.pc_].~T();

        // Decrement the producer count to match.
        if (!n_.compare_exchange_strong(n, {n.pc_ - 1, n.cc_}, std::memory_order_release, std::memory_order_relaxed)) abort();
        return res;
    }
    std::optional<T> top() const {
        // Decrement the consumer count.
        counts_t n = n_.load(std::memory_order_relaxed);
        if (n.cc_ == 0) return std::optional<T>(std::nullopt);
        int i = 0;
        while (!n.equal(n_) || !n_.compare_exchange_weak(n, {n.pc_, n.cc_ - 1}, std::memory_order_acquire, std::memory_order_relaxed)) { 
            if (n.cc_ == 0) return std::optional<T>(std::nullopt);
            nanosleep(i);
        };
        // Consumer count decremented, slot n.cc_ - 1 is ours.
        --n.cc_;
        std::optional<T> res(std::move(s_[n.pc_]));

        // Restore the consumer count.
        if (!n_.compare_exchange_strong(n, {n.pc_, n.cc_ + 1}, std::memory_order_release, std::memory_order_relaxed)) abort();
        return res;
    }
    void reset() { ::reset(s_); s_.resize(cap_); }
};

mt_stack<int> s;

void BM_stack(benchmark::State& state) {
  if (state.thread_index == 0) s.reset();
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) s.push(i);
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stack_top(benchmark::State& state) {
  if (state.thread_index == 0) s.reset();
  const size_t N = state.range(0);
  if (state.thread_index == 0) {
    for (size_t i = 0; i < N; ++i) s.push(i);
  }
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s.top());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stack_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) s.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) s.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->Arg(1) \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_stack) ARGS;
BENCHMARK(BM_stack_top) ARGS;
BENCHMARK(BM_stack_prod_cons) ARGS;

BENCHMARK_MAIN();
