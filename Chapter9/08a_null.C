extern void g();
int f(int* p) {
    if (p) g();
    return *p;
}

