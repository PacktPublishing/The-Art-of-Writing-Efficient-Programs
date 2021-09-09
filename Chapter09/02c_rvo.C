#include <iostream>
using namespace std;

class C {
    int i_ = 0;
    public:
    explicit C(int i) : i_(i) { cout << "C() @" << this << endl; }
    C(const C& c) = delete;
    C(C&& c) = delete;
    ~C() { cout << "~C() @" << this << endl; }
    friend ostream& operator<<(ostream& out, const C& c) { out << c.i_; return out; }
};

C makeC(int i) { return C(i); }

int main() {
    C c = makeC(42);
    cout << c << endl;
}
