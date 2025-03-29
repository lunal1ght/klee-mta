#include <pthread.h>
#include <stdio.h>
#include <klee/klee.h>

// Глобальная переменная, к которой будут обращаться потоки
int shared_counter = 0;

// Символическая переменная для демонстрации
// (В этом простом примере она не влияет на гонку, но показывает использование)
int sym_factor;

// Функция, выполняемая каждым потоком
void *worker_thread(void *arg) {
    int thread_id = *(int*)arg;

    // --- Начало критической секции (но без блокировки!) ---
    // Потенциальное состояние гонки:
    // Поток 1 читает shared_counter (например, 0)
    // KLEE может переключить контекст на Поток 2
    // Поток 2 читает shared_counter (тоже 0)
    // Поток 2 инкрементирует и записывает (shared_counter становится 1)
    // KLEE может переключить контекст обратно на Поток 1
    // Поток 1 инкрементирует свое прочитанное значение (0 -> 1) и записывает (shared_counter снова становится 1)
    // Ожидалось: 2, Получилось: 1 - "потерянное обновление"

    int temp = shared_counter;
    temp = temp + 1; // Инкремент
    shared_counter = temp; // Запись обратно
    // --- Конец критической секции (без блокировки!) ---

    printf("Thread %d finished incrementing. Counter is now (potentially racy): %d\n", thread_id, shared_counter);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;

    // Сделаем переменную символической (для примера)
    klee_make_symbolic(&sym_factor, sizeof(sym_factor), "sym_factor");

    printf("Main thread: starting counter = %d\n", shared_counter);

    // Создаем два потока
    if (pthread_create(&thread1, NULL, worker_thread, &id1)) {
        perror("Error creating thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, worker_thread, &id2)) {
        perror("Error creating thread 2");
        return 1; // Или дождаться первого потока перед выходом
    }

    // Ждем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Main thread: final counter = %d\n", shared_counter);

    // Проверяем финальное значение счетчика с помощью KLEE
    // Если произошла гонка (lost update), значение будет 1, а не 2.
    // KLEE должен найти путь выполнения, где это утверждение неверно.
    klee_assert(shared_counter == 2); // Ожидаем 2, если гонки не было

    // Пример использования символической переменной
    if (sym_factor > 100) {
         // Этот код будет исследован KLEE, если решатель найдет
         // значение sym_factor > 100
         printf("Symbolic factor was > 100\n");
    } else {
         printf("Symbolic factor was <= 100\n");
    }


    printf("Main thread: Assertion passed or KLEE found a counterexample.\n");
    return 0;
}
