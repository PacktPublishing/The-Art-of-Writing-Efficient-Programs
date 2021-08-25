#include <iostream>
using namespace std;

bool f1(int* a, int* b) {
    if (*a == *b) {
        ++*a;
        return *a == *b + 1;
    } else { 
        return true;
    }
}

bool f2(int* a, int* b) {
    if (*a == *b) ++*a;
    return true;
}

int main() {
    int a = 1, b = 2;
    cout << f1(&a, &b); cout << " " << a << " " << b << endl;
    cout << f1(&a, &a); cout << " " << a << " " << b << endl;
        a = 1, b = 2;
    cout << f2(&a, &b); cout << " " << a << " " << b << endl;
    cout << f2(&a, &a); cout << " " << a << " " << b << endl;
}
