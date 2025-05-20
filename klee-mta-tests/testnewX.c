#include <pthread.h>
#include <stdio.h>
#include <klee/klee.h>

// Глобальная переменная, к которой будут обращаться потоки
volatile int shared_counter = 0;
int num_increments_global;

// ---> НОВОЕ: Объявляем мьютекс <---
pthread_mutex_t counter_lock;

void *worker_thread(void *arg) {
    for (int i = 0; i < num_increments_global; ++i) {
        // ---> НОВОЕ: Захватываем мьютекс перед доступом к shared_counter <---
        pthread_mutex_lock(&counter_lock);

        // Критическая секция ТЕПЕРЬ ЗАЩИЩЕНА
        int temp = shared_counter;
        temp = temp + 1;
        shared_counter = temp;

        // ---> НОВОЕ: Освобождаем мьютекс после доступа <---
        pthread_mutex_unlock(&counter_lock);
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;

    // ---> НОВОЕ: Инициализируем мьютекс <---
    if (pthread_mutex_init(&counter_lock, NULL) != 0) {
        perror("Mutex init failed");
        return 1;
    }

    klee_make_symbolic(&num_increments_global, sizeof(num_increments_global), "num_increments");
    klee_assume(num_increments_global >= 1);
    klee_assume(num_increments_global <= 3);

    printf("Main: KLEE will explore with num_increments between 1 and 3.\n");
    printf("Main: Initial shared_counter = %d\n", shared_counter);

    if (pthread_create(&thread1, NULL, worker_thread, &id1)) { /* ... */ return 1;}
    if (pthread_create(&thread2, NULL, worker_thread, &id2)) { /* ... */ return 1;}

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Main: Final shared_counter = %d\n", shared_counter);
    printf("Main: For this path, num_increments_global was (concretized by KLEE): %d\n", num_increments_global);

    // Теперь этот assert НЕ должен срабатывать, так как гонки нет
    klee_assert_race(shared_counter == (2 * num_increments_global),
                     &shared_counter, sizeof(shared_counter), "shared_counter");

    printf("Main: Assertion check passed or KLEE found a counterexample.\n");

    // ---> НОВОЕ: Уничтожаем мьютекс <---
    pthread_mutex_destroy(&counter_lock);

    return 0;
}