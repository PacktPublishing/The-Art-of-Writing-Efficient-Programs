#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <vector>
#include <list>
#include <algorithm>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_read_vector(benchmark::State& state) {
    const size_t size = state.range(0);
    std::vector<Word> c(size);
    {
        Word x = {};
        for (auto it = c.begin(), it0 = c.end(); it != it0; ++it) *it = ++x;
    }

    for (auto _ : state) {
        for (auto it = c.begin(), it0 = c.end(); it != it0; ) {
            REPEAT(benchmark::DoNotOptimize(*it++);)
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(size*sizeof(Word)*state.iterations());
    state.SetItemsProcessed(size*state.iterations());
    char buf[1024];
    snprintf(buf, sizeof(buf), "%lu", size);
    state.SetLabel(buf);
}

template <class Word>
void BM_write_vector(benchmark::State& state) {
    const size_t size = state.range(0);
    std::vector<Word> c(size);
    Word x = {};

    for (auto _ : state) {
        for (auto it = c.begin(), it0 = c.end(); it != it0; ) {
            REPEAT(benchmark::DoNotOptimize(*it++ = x);)
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(size*sizeof(Word)*state.iterations());
    state.SetItemsProcessed(size*state.iterations());
    char buf[1024];
    snprintf(buf, sizeof(buf), "%lu", size);
    state.SetLabel(buf);
}

template <class Word>
void BM_read_list(benchmark::State& state) {
    const size_t size = state.range(0);
    std::list<Word> c;
    {
        Word x = {};
        for (size_t i = 0; i < size; ++i) c.push_back(x++);
    }

    for (auto _ : state) {
        for (auto it = c.begin(), it0 = c.end(); it != it0; ) {
            REPEAT(benchmark::DoNotOptimize(*it++);)
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(size*sizeof(Word)*state.iterations());
    state.SetItemsProcessed(size*state.iterations());
    char buf[1024];
    snprintf(buf, sizeof(buf), "%lu", size);
    state.SetLabel(buf);
}

template <class Word>
void BM_write_list(benchmark::State& state) {
    const size_t size = state.range(0);
    std::list<Word> c(size);
    Word x = {};

    for (auto _ : state) {
        for (auto it = c.begin(), it0 = c.end(); it != it0; ) {
            REPEAT(benchmark::DoNotOptimize(*it++ = x);)
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(size*sizeof(Word)*state.iterations());
    state.SetItemsProcessed(size*state.iterations());
    char buf[1024];
    snprintf(buf, sizeof(buf), "%lu", size);
    state.SetLabel(buf);
}

#define ARGS \
    ->Arg(1<<20)
    //->RangeMultiplier(2)->Range(1<<6, 1<<16)

//BENCHMARK_TEMPLATE1(BM_read_vector, unsigned int) ARGS;
BENCHMARK_TEMPLATE1(BM_read_vector, unsigned long) ARGS;
BENCHMARK_TEMPLATE1(BM_read_list, unsigned long) ARGS;
BENCHMARK_TEMPLATE1(BM_write_vector, unsigned long) ARGS;
BENCHMARK_TEMPLATE1(BM_write_list, unsigned long) ARGS;
//BENCHMARK_TEMPLATE1(BM_read_vector, __m128i) ARGS;
//BENCHMARK_TEMPLATE1(BM_read_vector, __m256i) ARGS;

BENCHMARK_MAIN();
