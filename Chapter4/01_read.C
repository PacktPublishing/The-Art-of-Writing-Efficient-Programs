#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <vector>
#include <algorithm>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_read_rand(benchmark::State& state) {
    volatile Word* const p = new Word;
    ::memset((void*)p, 0xab, sizeof(Word));

    for (auto _ : state) {
        REPEAT(benchmark::DoNotOptimize(*p);)
    }
    delete p;
    state.SetBytesProcessed(32*sizeof(Word)*state.iterations());
    state.SetItemsProcessed(32*state.iterations());
}


BENCHMARK_TEMPLATE1(BM_read_rand, unsigned int);
BENCHMARK_TEMPLATE1(BM_read_rand, unsigned long);
BENCHMARK_TEMPLATE1(BM_read_rand, __m128i);
BENCHMARK_TEMPLATE1(BM_read_rand, __m256i);

BENCHMARK_MAIN();
