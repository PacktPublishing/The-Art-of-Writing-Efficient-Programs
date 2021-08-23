#include <stdlib.h>
#include <string.h>
#include <deque>

#include "benchmark/benchmark.h"

template<typename T>
struct compare_ptr {
    bool operator()(const T* a, const T* b) const { return *a < *b; }
};

template <typename T>
class index_tree {
    using idx_t = typename std::set<T*, compare_ptr<T>>;
    using idx_iter_t = typename idx_t::const_iterator;
    public:
    index_tree(size_t N) { data_.reserve(N); }
    void insert(const T& t) { data_.push_back(t); idx_.insert(&(data_[data_.size() - 1])); }
    class const_iterator {
        idx_iter_t it_;
        public:
        const_iterator(idx_iter_t it) : it_(it) {}
        const_iterator operator++() { ++it_; return *this; }
        const T& operator*() const { return *(*it_); }
        friend bool operator!=(const const_iterator& a, const const_iterator& b) { return a.it_ != b.it_; }
    };
    const_iterator cbegin() const { return idx_.cbegin(); }
    const_iterator cend() const { return idx_.cend(); }
    template <typename F> bool find(F f) const {
        for (const T& x : data_) {
            if (f(x)) return true;
        }
        return false;
    }
    private:
    std::set<T*, compare_ptr<T>> idx_;
    std::vector<T> data_;
};

template <typename C, typename F>
bool find(const C& c, F f) {
    for (auto it = c.cbegin(), it0 = c.cend(); it != it0; ++it) {
        if (f(*it)) return true;
    }
    return false;
}

#include <iostream>
using namespace std;
void BM_iter(benchmark::State& state) {
    const unsigned int N = state.range(0);
    index_tree<unsigned long> t(N);
    for (size_t i = 0; i < N; ++i) {
        t.insert(rand());
    }
    for (auto _ : state) {
        const bool found = find(t, [](unsigned long x) { return x == 0; });
        benchmark::DoNotOptimize(found);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_find(benchmark::State& state) {
    const unsigned int N = state.range(0);
    index_tree<unsigned long> t(N);
    for (size_t i = 0; i < N; ++i) {
        t.insert(rand());
    }
    for (auto _ : state) {
        bool found = t.find([](unsigned long x) { return x == 0; });
        benchmark::DoNotOptimize(found);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

#define ARGS \
    ->Arg(1<<12)

BENCHMARK(BM_iter) ARGS;
BENCHMARK(BM_find) ARGS;

BENCHMARK_MAIN();

