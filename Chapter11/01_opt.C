#include <iostream>
#include <limits.h>

using namespace std;

int main() {
    cout << INT_MAX << ": ";
    int k;
    cin >> k;
    if (k > INT_MAX-5) cout << "Large k" << endl;
    if (k + 10 < INT_MAX) cout << "Small k" << endl;
}
