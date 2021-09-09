#include <unistd.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <execution>

#include "benchmark/benchmark.h"

auto work1 = [](double& x){ ++x; };
auto work2 = [](double& x){ x = sin(x) + cos(x)*exp(-x); };

void BM_foreach(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(v.begin(), v.end(), work1);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_seq(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::seq, v.begin(), v.end(), work1);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_par(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::par, v.begin(), v.end(), work1);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_par_unseq(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::par_unseq, v.begin(), v.end(), work1);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_unseq(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::unseq, v.begin(), v.end(), work1);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_compute(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(v.begin(), v.end(), work2);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_seq_compute(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::seq, v.begin(), v.end(), work2);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_par_compute(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::par, v.begin(), v.end(), work2);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_par_unseq_compute(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::par_unseq, v.begin(), v.end(), work2);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_foreach_unseq_compute(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::for_each(std::execution::unseq, v.begin(), v.end(), work2);
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_sort_seq(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::sort(std::execution::seq, v.begin(), v.end());
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_sort_par(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<double> v(N);
    std::for_each(v.begin(), v.end(), [](double& x){ x = rand(); });
    for (auto _ : state) {
        std::sort(std::execution::par, v.begin(), v.end());
    }
    state.SetItemsProcessed(N*state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);
#define ARG \
    ->Arg(1UL<<10) \
    ->Arg(1UL<<15) \
    ->UseRealTime() \
    ->ThreadRange(1, 1)

BENCHMARK(BM_foreach) ARG;
BENCHMARK(BM_foreach_seq) ARG;
BENCHMARK(BM_foreach_par) ARG;
//BENCHMARK(BM_foreach_par_unseq) ARG;
//BENCHMARK(BM_foreach_unseq) ARG;
BENCHMARK(BM_foreach_compute) ARG;
BENCHMARK(BM_foreach_seq_compute) ARG;
BENCHMARK(BM_foreach_par_compute) ARG;
//BENCHMARK(BM_foreach_par_unseq_compute) ARG;
//BENCHMARK(BM_foreach_unseq_compute) ARG;
BENCHMARK(BM_sort_seq) ARG;
BENCHMARK(BM_sort_par) ARG;

BENCHMARK_MAIN();
