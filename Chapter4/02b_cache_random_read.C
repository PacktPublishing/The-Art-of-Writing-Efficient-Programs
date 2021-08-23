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
    void* memory;
    const size_t size = state.range(0);
    if (::posix_memalign(&memory, 64, size) != 0) abort();
    void* const end = static_cast<char*>(memory) + size;
    volatile Word* const p0 = static_cast<Word*>(memory);
    Word* const p1 = static_cast<Word*>(end);
    Word sink1; ::memset(&sink1, 0xab, sizeof(sink1));
    Word sink = sink1;

    const size_t N = size/sizeof(Word);
    std::vector<int> v_index(N); 
    for (size_t i = 0; i < N; ++i) v_index[i] = i;
    std::random_shuffle(v_index.begin(), v_index.end());
    int* const index = v_index.data();
    int* const i1 = index + N;

    for (auto _ : state) {
        for (const int* ind = index; ind < i1; ) {
            //REPEAT(benchmark::DoNotOptimize(sink = *(p0 + *ind++));)
            REPEAT(benchmark::DoNotOptimize(*(p0 + *ind++));)
        }
        benchmark::ClobberMemory();
    }
    benchmark::DoNotOptimize(sink);
    ::free(memory);
    state.SetBytesProcessed(size*state.iterations());
    state.SetItemsProcessed((p1 - p0)*state.iterations());
    char buf[1024];
    snprintf(buf, sizeof(buf), "%lu", size);
    state.SetLabel(buf);
}


BENCHMARK_TEMPLATE1(BM_read_rand, unsigned long)->Arg(1<<30);

BENCHMARK_MAIN();
