#include <cstring>
#include <iostream>
using namespace std;

void init(char* a, char* b, size_t N) {
    for (size_t i = 0; i < N; ++i) {
        a[i] = '0';
        b[i] = '1';
    }
}

void init1(char* a, char* b, size_t N) {
    std::memset(a, '0', N);
    std::memset(b, '1', N);
}

int main() {
    char a[10], b[10];

    std::memset(a, '-', 10);
    init(a, b, 10);
    for (char i : a) cout << i << " ";
    cout << endl;

    std::memset(a, '-', 10);
    init(a, a + 2, 4);
    for (char i : a) cout << i << " ";
    cout << endl;

    std::memset(a, '-', 10);
    init(a, a + 5, 4);
    for (char i : a) cout << i << " ";
    cout << endl;

    std::memset(a, '-', 10);
    init1(a, b, 10);
    for (char i : a) cout << i << " ";
    cout << endl;

    std::memset(a, '-', 10);
    init1(a, a + 2, 4);
    for (char i : a) cout << i << " ";
    cout << endl;

    std::memset(a, '-', 10);
    init1(a, a + 5, 4);
    for (char i : a) cout << i << " ";
    cout << endl;
}
