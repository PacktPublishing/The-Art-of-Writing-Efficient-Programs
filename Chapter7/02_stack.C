#include <optional>
#include <stack>
#include <mutex>
#include <shared_mutex>
#include <unistd.h>

#include "spinlock.h"

#include "benchmark/benchmark.h"

template <typename T> void reset(T& s) { T().swap(s); }

template <typename T> class st_stack
{
    std::stack<T> s_;
    public:
    void push(const T& v) {
        s_.push(v);
    }
    std::optional<T> pop() {
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(s_.top()));
            s_.pop();
            return res;
        }
    }
    std::optional<T> top() const {
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(s_.top());
            return res;
        }
    }
    void reset() { ::reset(s_); }
};

template <typename T> class mt_stack
{
    std::stack<T> s_;
    mutable spinlock l_;
    public:
    void push(const T& v) {
        std::lock_guard g(l_);
        s_.push(v);
    }
    std::optional<T> pop() {
        std::lock_guard g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(s_.top()));
            s_.pop();
            return res;
        }
    }
    std::optional<T> top() const {
        std::lock_guard g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(s_.top());
            return res;
        }
    }
    void reset() { ::reset(s_); }
};

template <typename T> class mt_stack1
{
    std::stack<T> s_;
    mutable std::mutex l_;
    public:
    void push(const T& v) {
        std::lock_guard g(l_);
        s_.push(v);
    }
    void push(const T& v, size_t N) {
        std::lock_guard g(l_);
        for (size_t i = 0; i != N; ++i) s_.push(v);
    }
    std::optional<T> pop() {
        std::lock_guard g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(s_.top()));
            s_.pop();
            return res;
        }
    }
    std::optional<T> pop(size_t N) {
        std::lock_guard g(l_);
        for (size_t i = 0; ;) {
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(s_.top()));
            s_.pop();
            if (++i == N) return res;
        }
        }
    }
    std::optional<T> top() const {
        std::lock_guard g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(s_.top());
            return res;
        }
    }
    void reset() { ::reset(s_); }
};

template <typename T> class rw_stack
{
    std::stack<T> s_;
    mutable rw_spinlock l_;
    public:
    void push(const T& v) {
        std::unique_lock g(l_);
        s_.push(v);
    }
    std::optional<T> pop() {
        std::unique_lock g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(s_.top()));
            s_.pop();
            return res;
        }
    }
    std::optional<T> top() const {
        std::shared_lock g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(s_.top());
            return res;
        }
    }
    void reset() { ::reset(s_); }
};

template <typename T> class rw_stack1
{
    std::stack<T> s_;
    mutable std::shared_mutex l_;
    public:
    void push(const T& v) {
        std::unique_lock g(l_);
        s_.push(v);
    }
    std::optional<T> pop() {
        std::unique_lock g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(s_.top()));
            s_.pop();
            return res;
        }
    }
    std::optional<T> top() const {
        std::shared_lock g(l_);
        if (s_.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(s_.top());
            return res;
        }
    }
    void reset() { ::reset(s_); }
};

void BM_stack0(benchmark::State& state) {
  st_stack<int> s0;
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) s0.push(i);
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s0.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

std::atomic<size_t> n;
void BM_stack0_inc(benchmark::State& state) {
  st_stack<int> s0;
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) {
        n.fetch_add(1, std::memory_order_release);
        s0.push(i);
    }
    for (size_t i = 0; i < N; ++i) {
        n.fetch_sub(1, std::memory_order_acquire);
        benchmark::DoNotOptimize(s0.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stack0_cas(benchmark::State& state) {
  st_stack<int> s0;
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) {
        size_t nn = n.load(std::memory_order_relaxed);
        n.compare_exchange_weak(nn, nn, std::memory_order_release, std::memory_order_relaxed);
        s0.push(i);
    }
    for (size_t i = 0; i < N; ++i) {
        size_t nn = n.load(std::memory_order_relaxed);
        n.compare_exchange_weak(nn, nn, std::memory_order_acquire, std::memory_order_relaxed);
        benchmark::DoNotOptimize(s0.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

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

mt_stack1<int> s1;

void BM_stack1(benchmark::State& state) {
  if (state.thread_index == 0) s1.reset();
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) s1.push(i);
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s1.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stackN1(benchmark::State& state) {
  if (state.thread_index == 0) s1.reset();
  const size_t N = state.range(0);
  for (auto _ : state) {
    s1.push(1, N);
    benchmark::DoNotOptimize(s1.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

rw_stack<int> srw;

void BM_stackrw(benchmark::State& state) {
  if (state.thread_index == 0) srw.reset();
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) srw.push(i);
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(srw.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

rw_stack1<int> srw1;

void BM_stackrw1(benchmark::State& state) {
  if (state.thread_index == 0) srw1.reset();
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) srw1.push(i);
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(srw1.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stack0_top(benchmark::State& state) {
  st_stack<int> s0;
  const size_t N = state.range(0);
  if (state.thread_index == 0) {
    for (size_t i = 0; i < N; ++i) s0.push(i);
  }
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s0.top());
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

void BM_stack1_top(benchmark::State& state) {
  if (state.thread_index == 0) s1.reset();
  const size_t N = state.range(0);
  if (state.thread_index == 0) {
    for (size_t i = 0; i < N; ++i) s1.push(i);
  }
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s1.top());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stackrw_top(benchmark::State& state) {
  if (state.thread_index == 0) srw.reset();
  const size_t N = state.range(0);
  if (state.thread_index == 0) {
    for (size_t i = 0; i < N; ++i) srw.push(i);
  }
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(srw.top());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stackrw1_top(benchmark::State& state) {
  if (state.thread_index == 0) srw1.reset();
  const size_t N = state.range(0);
  if (state.thread_index == 0) {
    for (size_t i = 0; i < N; ++i) srw1.push(i);
  }
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(srw1.top());
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

void BM_stack1_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) s1.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) s1.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s1.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stackrw_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) srw.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) srw.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(srw.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_stackrw1_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) srw1.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) srw1.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(srw1.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->Arg(1) \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

//BENCHMARK(BM_stack0) ARGS;
BENCHMARK(BM_stack) ARGS;
BENCHMARK(BM_stack1) ARGS;
//BENCHMARK(BM_stackN1) ARGS;
BENCHMARK(BM_stackrw) ARGS;
//BENCHMARK(BM_stackrw1) ARGS;
//BENCHMARK(BM_stack0_inc) ARGS;
//BENCHMARK(BM_stack0_cas) ARGS;
//BENCHMARK(BM_stack0_top) ARGS;
BENCHMARK(BM_stack_top) ARGS;
BENCHMARK(BM_stack1_top) ARGS;
BENCHMARK(BM_stackrw_top) ARGS;
//BENCHMARK(BM_stackrw1_top) ARGS;

BENCHMARK(BM_stack_prod_cons) ARGS;
BENCHMARK(BM_stack1_prod_cons) ARGS;
BENCHMARK(BM_stackrw_prod_cons) ARGS;
//BENCHMARK(BM_stackrw1_prod_cons) ARGS;

BENCHMARK_MAIN();
