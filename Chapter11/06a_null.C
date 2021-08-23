int f(int* p) {
    ++(*p);
    return p ? *p : 0;
}

