// Benchmark for comparison functions used in 01-04, with repetitions.
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;
using std::cout;
using std::endl;
using std::unique_ptr;

bool compare1(const char* s1, const char* s2) {
    int i1 = 0, i2 = 0;
    char c1, c2;
    while (1) {
        c1 = s1[i1]; c2 = s2[i2];
        if (c1 != c2) return c1 > c2;
        ++i1; ++i2;
    }
}

bool compare2(const char* s1, const char* s2) {
    unsigned int i1 = 0, i2 = 0;
    char c1, c2;
    while (1) {
        c1 = s1[i1]; c2 = s2[i2];
        if (c1 != c2) return c1 > c2;
        ++i1; ++i2;
    }
}

int main() {
    constexpr unsigned int N = 1 << 20;
    constexpr int NI = 1 << 11;
    unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    system_clock::time_point t0 = system_clock::now();
    for (int i = 0; i < NI; ++i) {
        compare1(s.get(), s.get() + N);
    }
    system_clock::time_point t1 = system_clock::now();
    for (int i = 0; i < NI; ++i) {
        compare2(s.get(), s.get() + N);
    }
    system_clock::time_point t2 = system_clock::now();
    cout << duration_cast<microseconds>(t1 - t0).count() << "us " << duration_cast<microseconds>(t2 - t1).count() << "us" << endl;
}
