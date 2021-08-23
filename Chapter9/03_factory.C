#include <memory>
#include <iostream>
using namespace std;

class C {
    int i_ = 0;
    public:
    explicit C(int i) : i_(i) { cout << "C() @" << this << endl; }
    C(const C& c) : i_(c.i_) { cout << "C(const C&) @" << this << endl; }
    C(C&& c) : i_(c.i_) { cout << "C(C&&) @" << this << endl; }
    ~C() { cout << "~C() @" << this << endl; }
    friend ostream& operator<<(ostream& out, const C& c) { out << c.i_; return out; }
};

std::unique_ptr<C> makeC(int i) { return make_unique<C>(i); }

int main() {
    auto c = makeC(42);
    cout << *c << endl;
}
