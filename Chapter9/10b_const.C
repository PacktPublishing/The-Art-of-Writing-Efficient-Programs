void g(int y);
bool f(int x) {
    const int y = x + 1;
    g(y);
    return y > x;
}
