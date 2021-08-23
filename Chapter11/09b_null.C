extern int g(int*);
int f(int* p) {
    ++(*p);
    return g(p);
}

