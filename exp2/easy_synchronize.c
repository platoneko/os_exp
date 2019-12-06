#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int a = 0;
sem_t wrt_s, read_s;

void *thread_routine_1() {
    for (int i = 1; i <= 100; ++i) {
        sem_wait(&wrt_s);
        a += i;
        sem_post(&read_s);
        usleep(1000);
    }
    return NULL;
}

void *thread_routine_2() {
    for (int i = 1; i <= 100; ++i) {
        sem_wait(&read_s);
        printf("%d\n", a);
        fflush(stdout);
        sem_post(&wrt_s);
        usleep(1000);
    }
    return NULL;
}

int main() {
    sem_init(&wrt_s, 0, 1);
    sem_init(&read_s, 0, 0);
    pthread_t tid_1, tid_2;
    pthread_create(&tid_1, NULL, thread_routine_1, NULL);
    pthread_create(&tid_2, NULL, thread_routine_2, NULL);
    pthread_join(tid_1, NULL);
    pthread_join(tid_2, NULL);
    sem_close(&wrt_s);
    sem_close(&read_s);
    exit(0);
}
