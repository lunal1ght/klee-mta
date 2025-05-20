#include <pthread.h>
#include <stdio.h>
#include <klee/klee.h>

// Глобальная переменная, к которой будут обращаться потоки
volatile int shared_counter = 0; // volatile, чтобы компилятор не оптимизировал лишнего

// Глобальная символическая переменная для количества инкрементов
int num_increments_global=1;

// Функция, выполняемая каждым потоком
void *worker_thread(void *arg) {
    // Каждый поток работает со своей копией num_increments_global,
    // но так как она глобальная и символическая, KLEE будет рассматривать
    // одно и то же символическое значение для нее на разных путях.
    for (int i = 0; i < num_increments_global; ++i) {
        // Критическая секция без блокировки
        int temp = shared_counter;
        // Здесь KLEE может переключить контекст, особенно если num_increments_global > 1
        temp = temp + 1;
        shared_counter = temp;
    }
    // printf("Thread finished. Counter (potentially racy): %d, Num_increments was: %d\n", shared_counter, num_increments_global);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2; // Аргументы для потоков (в этом примере не используются)

    // Делаем количество инкрементов символическим
//    klee_make_symbolic(&num_increments_global, sizeof(num_increments_global), "num_increments");

    // Ограничиваем диапазон символических значений для num_increments_global
    // чтобы анализ был более управляемым и для демонстрации klee_assume.
    // KLEE будет исследовать только пути, где это условие истинно.
    // Если num_increments_global == 0, цикл не выполнится, assert будет 0 == 0 (пройдет).
    // Мы хотим, чтобы циклы выполнялись.
//    klee_assume(num_increments_global >= 1);
//    klee_assume(num_increments_global <= 3); // Ограничим до 3 для скорости

    printf("Main: KLEE will explore with num_increments between 1 and 3.\n");
    printf("Main: Initial shared_counter = %d\n", shared_counter);

    // Создаем два потока
    if (pthread_create(&thread1, NULL, worker_thread, &id1)) {
        perror("Error creating thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, worker_thread, &id2)) {
        perror("Error creating thread 2");
        return 1;
    }

    // Ждем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Main: Final shared_counter = %d\n", shared_counter);
    printf("Main: For this path, num_increments_global was (concretized by KLEE): %d\n", num_increments_global);

    // Проверяем финальное значение счетчика
    // Ожидаемое значение, ЕСЛИ бы не было гонки, это 2 * num_increments_global
    // KLEE должен найти пути (комбинации num_increments_global и порядка выполнения),
    // где это утверждение неверно из-за гонки.
//    klee_assert(shared_counter == (2 * num_increments_global));
    klee_assert_race(shared_counter == (2 * num_increments_global), 
                 &shared_counter, 
                 sizeof(shared_counter), 
                 "shared_counter");

    printf("Main: Assertion check passed for this path or KLEE found a counterexample.\n");
    return 0;
}
