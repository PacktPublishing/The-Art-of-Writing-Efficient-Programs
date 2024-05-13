## Building instructions:
For .C files that have a comment like the following

```
// Build as follows:
// $CXX 01_substring_sort.C 01_substring_sort_a.C -I$GBENCH_DIR/include -g -O3 -I. --std=c++17 $GBENCH_DIR/lib/libbenchmark.a -lpthread -latomic -lrt -lm -o 01_substring_sort
```

Build using these direction (feel free to add more warning and other flags). Do
not try to build individual files like 01_substring_sort_a.C, they won't link by
themselves. If the file has no such comment, then the file compiles and links
standalone. 

$CXX is the compiler, usually GCC or CLANG (Visual Studio will work but the
options have different syntax).

$GBENCH_DIR is the path to the Google Benchmark, to the level where include/ and
lib/ are dubdirectories. Library name may vary depending on how the benchmark is
installed.
