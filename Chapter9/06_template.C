#include <iostream>
#include <vector>
using namespace std;

enum op_t { do_shrink, do_grow };
class Shape {
    public:
    void shrink() {}
    void grow() {}
};

template <op_t op>
void process(std::vector<Shape>& v) {
    for (Shape& s : v) {
        if (op == do_shrink) s.shrink();
        else s.grow();
    }
}
void process(std::vector<Shape>& v, op_t op) {
    if (op == do_shrink) process<do_shrink>(v);
    else process<do_grow>(v);
}

int main() {
    std::vector<Shape> v(10);
    process(v, do_shrink);
}
