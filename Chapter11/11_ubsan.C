#include <iostream>
#include <limits.h>

using namespace std;

int main() {
    cout << INT_MAX << ": ";
    int k;
    cin >> k;
    bool large = k + 10 < INT_MAX;
    cout << "Large k? " << large << endl;
}
