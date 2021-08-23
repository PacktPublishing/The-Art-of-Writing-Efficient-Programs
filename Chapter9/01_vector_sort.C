#include <cstdlib>
#include <vector>
#include <iostream>

#include "benchmark/benchmark.h"

template <typename T> void sort1(std::vector<T> v) {
    std::sort(v.begin(), v.end());
    if (rand() < 0) for (T x: v) std::cout << x << std::endl; // Never happens but the compiler doesn't know that!
    //for (size_t i = 1; i < v.size(); ++i) if (v[i-1] > v[i]) abort();
}

template <typename T> void sort2(const std::vector<T>& v) {
    std::vector<const T*> vp; vp.reserve(v.size());
    for (const T& x: v) vp.push_back(&x);
    std::sort(vp.begin(), vp.end(), [](const T* a, const T* b) { return *a < *b; });
    if (rand() < 0) for (const T* p: vp) std::cout << *p << std::endl;
    //for (size_t i = 1; i < v.size(); ++i) if (*vp[i-1] > *vp[i]) abort();
}

void BM_copy_vec_int(benchmark::State& state) {
  const size_t N = state.range(0);
  std::vector<int> v0(N); for (int& x: v0) x = rand();
  std::vector<int> v(N);
  for (auto _ : state) {
      v = v0;
      if (rand() < 0) for (int x: v) std::cout << x << std::endl;
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_sort_vec_int(benchmark::State& state) {
  const size_t N = state.range(0);
  std::vector<int> v0(N); for (int& x: v0) x = rand();
  std::vector<int> v(N);
  for (auto _ : state) {
      v = v0;
      sort1(v);
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_sort_vec_intp(benchmark::State& state) {
  const size_t N = state.range(0);
  std::vector<int> v0(N); for (int& x: v0) x = rand();
  std::vector<int> v(N);
  for (auto _ : state) {
      v = v0;
      sort2(v);
  }
  state.SetItemsProcessed(state.iterations()*N);
}

struct S {
    enum { N = 128 };
    int a[N];
    S() { for (int& x: a) x = rand(); }
    friend bool operator<(const S& a, const S& b) {
        for (size_t i = 0; i < N; ++i) {
            if (a.a[i] == b.a[i]) continue;
            return a.a[i] < b.a[i];
        }
        return false;
    }
    friend std::ostream& operator<<(std::ostream& out, const S& x) {
        for (size_t i = 0; i < N; ++i) out << x.a[i] << " ";
        return out;
    }
};

void BM_copy_vec_S(benchmark::State& state) {
  const size_t N = state.range(0);
  std::vector<S> v0(N);
  std::vector<S> v(N);
  for (auto _ : state) {
      v = v0;
      if (rand() < 0) for (S x: v) std::cout << x << std::endl;
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_sort_vec_S(benchmark::State& state) {
  const size_t N = state.range(0);
  std::vector<S> v0(N);
  std::vector<S> v(N);
  for (auto _ : state) {
      v = v0;
      sort1(v);
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_sort_vec_Sp(benchmark::State& state) {
  const size_t N = state.range(0);
  std::vector<S> v0(N);
  std::vector<S> v(N);
  for (auto _ : state) {
      v = v0;
      sort2(v);
  }
  state.SetItemsProcessed(state.iterations()*N);
}

#define ARG \
  ->RangeMultiplier(2) \
  ->Arg(1UL << 6) \
  ->Arg(1UL << 10) \
  ->Arg(1UL << 20) \
  ->UseRealTime()

/*
  ->Range(1UL << 2, 1UL << 20) \

*/

BENCHMARK(BM_copy_vec_int) ARG;
BENCHMARK(BM_sort_vec_int) ARG;
BENCHMARK(BM_sort_vec_intp) ARG;
BENCHMARK(BM_copy_vec_S) ARG;
BENCHMARK(BM_sort_vec_S) ARG;
BENCHMARK(BM_sort_vec_Sp) ARG;

BENCHMARK_MAIN();
