#include <klee/klee.h>
int main() {
  int x;
  klee_make_symbolic(&x, sizeof(x), "x");
    klee_assert(x != 42);
  return 0;
}
