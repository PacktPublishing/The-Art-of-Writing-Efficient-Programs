template <typename T>
void myswap(T* p, T* q) {
    T t = *p;
    *p = *q;
    *q = t;
}

void f(int* p, int* q) {
    if (p && q) myswap(p, q);
}
