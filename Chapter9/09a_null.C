extern int g(int*);
int f(int* p) {
    ++(*p);
    return p ? g(p) : 0;
}

