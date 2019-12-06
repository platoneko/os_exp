#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int tickets = 10000;
sem_t mtx;

void *sell_routine(void *i) {
    int id = *(int *)i;
    while (1) {
        sem_wait(&mtx);
        if (tickets > 0) {
            printf("No.%d seller sold ticket %d!\n", id, tickets--);
            fflush(stdout);
            sem_post(&mtx);
        }
        else {
            break;
        }
        usleep(1000);
    }
    sem_post(&mtx);
    return NULL;
}

int main(int argc,char *argv[]) {
    int thread_num = 10;
    char opt;
    while ((opt=getopt(argc,argv,"n:t:"))!=-1) {
        switch (opt) {
          case 'n':
            tickets = atoi(optarg);
            break;
          case 't':
            thread_num = atoi(optarg);
            break;
        }
    }
    sem_init(&mtx, 0, 1);
    pthread_t tid[thread_num];
    int *tid_ = malloc(thread_num * sizeof(int));

    for (int i = 0; i < thread_num; ++i) {
        tid_[i] = i+1;
        pthread_create(&tid[i], NULL, sell_routine, &tid_[i]);
    }
    free(tid_);
    for (int i = 0; i < thread_num; ++i) {
        pthread_join(tid[i], NULL);
    }
    sem_close(&mtx);
    exit(0);
}
