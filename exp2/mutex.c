#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "../PV.h"

int tickets = 10000;
int mtx;

void *sell_routine(void *i) {
    int id = *(int *)i;
    while (1) {
        P(mtx, 0);
        if (tickets > 0) {
            printf("No.%d seller sold ticket %d!\n", id, tickets--);
            fflush(stdout);
            V(mtx, 0);
        }
        else {
            break;
        }
        usleep(1000);
    }
    V(mtx, 0);
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
    if ((mtx = semget(IPC_PRIVATE, 1, IPC_CREAT|0600)) == -1)
        exit(-1);
    semctl(mtx, 0, SETVAL, 1);
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
    semctl(mtx, 0, IPC_RMID);
    exit(0);
}
