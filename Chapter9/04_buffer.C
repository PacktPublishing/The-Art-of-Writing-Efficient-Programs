#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <vector>
#include "benchmark/benchmark.h"
using namespace std;

constexpr size_t nr = 1UL << 20;
std::vector<size_t> vr {
    [](size_t nr) { 
        std::vector<size_t> v;
        v.reserve(nr);
        srand(1);
        for (size_t i = 0; i < nr; ++i) v[i] = rand();
        return v;
    }(nr)
};

class Buffer {
    size_t size_;
    std::unique_ptr<char[]> buf_;
    public:
    explicit Buffer(size_t N) : size_(N), buf_(new char[N]) {}
    void resize(size_t N) { 
        if (N <= size_) return;
        char* new_buf = new char[N];
        memcpy(new_buf, get(), size_);
        buf_.reset(new_buf);
        size_ = N;
    }
    char* get() { return &buf_[0]; }
};


void BM_make_str_new(benchmark::State& state) {
  const size_t NMax = state.range(0);
  size_t ir = 0;
  for (auto _ : state) {
    const int r = vr[ir++ % nr];
    const size_t N = (r % NMax) + 1;
    char* buf = new char[N];
    memset(buf, 0xab, N);
    if (r < 0) cout << buf;
    delete [] buf;
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_make_str_max(benchmark::State& state) {
  const size_t NMax = state.range(0);
  char* buf = new char[NMax];
  size_t ir = 0;
  for (auto _ : state) {
    const int r = vr[ir++ % nr];
    const size_t N = (r % NMax) + 1;
    memset(buf, 0xab, N);
    if (r < 0) cout << buf;
  }
  delete [] buf;
  state.SetItemsProcessed(state.iterations());
}

void BM_make_str_buf(benchmark::State& state) {
  const size_t NMax = state.range(0);
  Buffer buf(1);
  size_t ir = 0;
  for (auto _ : state) {
    const int r = vr[ir++ % nr];
    const size_t N = (r % NMax) + 1;
    buf.resize(N);
    memset(buf.get(), 0xab, N);
    if (r < 0) cout << buf.get();
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARG \
  ->ThreadRange(1, numcpu) \
  ->Arg(1UL << 10) \
  ->UseRealTime()

BENCHMARK(BM_make_str_new) ARG;
BENCHMARK(BM_make_str_max) ARG;
BENCHMARK(BM_make_str_buf) ARG;

BENCHMARK_MAIN();
