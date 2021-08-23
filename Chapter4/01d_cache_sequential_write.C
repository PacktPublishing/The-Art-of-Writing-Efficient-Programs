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
void BM_write_seq(benchmark::State& state) {
    void* memory;
    const size_t size = state.range(0);
    if (::posix_memalign(&memory, 64, size) != 0) abort();
    void* const end = static_cast<char*>(memory) + size;
    volatile Word* const p0 = static_cast<Word*>(memory);
    Word* const p1 = static_cast<Word*>(end);
    Word fill1; ::memset(&fill1, 0xab, sizeof(fill1));
    Word fill = fill1;

    for (auto _ : state) {
        for (volatile Word* p = p0; p != p1; ) {
            REPEAT(*p++ = fill;)
        }
        benchmark::ClobberMemory();
    }
    benchmark::DoNotOptimize(fill);
    ::free(memory);
    state.SetBytesProcessed(size*state.iterations());
    state.SetItemsProcessed((p1 - p0)*state.iterations());
    char buf[1024];
    snprintf(buf, sizeof(buf), "%lu", size);
    state.SetLabel(buf);
}


#define ARGS \
    ->RangeMultiplier(2)->Range(1<<10, 1<<30)

BENCHMARK_TEMPLATE1(BM_write_seq, unsigned int) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, unsigned long) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, __m128i) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, __m256i) ARGS;

BENCHMARK_MAIN();
