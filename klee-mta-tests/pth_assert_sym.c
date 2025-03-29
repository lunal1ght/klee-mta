#include <pthread.h>
#include <klee/klee.h>

void* thread_func(void* arg) {
    int x = *(int*)arg;
    klee_assert(x != 0);  // Проверка KLEE на многопоточный ввод
    return NULL;
}

int main() {
    pthread_t thread;
    int x;
    klee_make_symbolic(&x, sizeof(x), "x");
    pthread_create(&thread, NULL, thread_func, &x);
    pthread_join(thread, NULL);
    return 0;
}

