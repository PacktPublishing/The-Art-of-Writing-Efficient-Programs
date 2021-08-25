#include <iostream>
#include <vector>
using namespace std;

class Shape {
    public:
};

template <bool use_length, bool use_width>
void measure(const std::vector<Shape>& v,
        double* length, double* width, double* depth,
        double* volume, double* weight) {
}
void measure(const std::vector<Shape>& v,
        double* length, double* width, double* depth,
        double* volume, double* weight) {
    const int key = ((length != nullptr) << 0) |
                    ((width  != nullptr) << 1) |
                    ((width  != nullptr) << 1) |
                    ((depth  != nullptr) << 1) |
                    ((volume != nullptr) << 1) |
                    ((weight != nullptr) << 1);
    switch (key) {
        case 0x01: measure<true , false>(v, length, width, depth, volume, weight);
                   break;
        case 0x02: measure<false, true >(v, length, width, depth, volume, weight);
                   break;
        case 0x03: measure<true , true >(v, length, width, depth, volume, weight);
                   break;
        default:; // assert
    }
    cout << key << endl;
}

int main() {
    std::vector<Shape> v(10);
    double l, w;
    measure(v, &l, &w, nullptr, nullptr, nullptr);
}
