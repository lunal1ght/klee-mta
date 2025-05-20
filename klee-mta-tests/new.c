#include <pthread.h>
#include <stdio.h>
#include <klee/klee.h>

// Глобальная переменная, вызывающая data race
int shared_counter = 0;

// Функция потока
void* thread_func(void* arg) {
    int i;
    for (i = 0; i < 5; ++i) {
        shared_counter += 1;
    }
    return NULL;
}

int main() {
    // Символическая переменная для активации символьного исполнения
    unsigned char symbolic_input;
    klee_make_symbolic(&symbolic_input, sizeof(symbolic_input), "symbolic_input");

    // Создаем два потока
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);

    // Ожидаем завершения потоков
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Проверяем результат
    klee_assert(shared_counter == 10);

    return 0;
}
