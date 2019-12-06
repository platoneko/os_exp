#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include "../PV.h"

int a = 0, wrt_s, read_s;

void *thread_routine_1() {
    for (int i = 1; i <= 100; ++i) {
        P(wrt_s, 0);
        a += i;
        V(read_s, 0);
        usleep(1000);
    }
    return NULL;
}

void *thread_routine_2() {
    for (int i = 1; i <= 100; ++i) {
        P(read_s, 0);
        printf("%d %d\n", i, a);
        fflush(stdout);
        V(wrt_s, 0);
        usleep(1000);
    }
    return NULL;
}

int main() {
    pthread_t tid_1, tid_2;

    /* 
        IPC_PRIVATE 会建立新信号量集对象 
        IPC_CREAT | IPC_EXCL 如果内核中不存在键值与key相等的信号量集，则新建一个消息队列；如果存在这样的信号量集则报错
    */
    if ((wrt_s = semget(IPC_PRIVATE, 1, IPC_CREAT|0600)) == -1) {
        fprintf(stderr, "wrt_s create error!\n");
        fflush(stderr);
        exit(-1);
    }
    if ((read_s = semget(IPC_PRIVATE, 1, IPC_CREAT|0600)) == -1) {
        fprintf(stderr, "read_s create error!\n");
        fflush(stderr);
        exit(-1);        
    }
    if (semctl(wrt_s, 0, SETVAL, 1) == -1) {
        fprintf(stderr, "wrt_s init error!\n");
        fflush(stderr);
        exit(-1);   
    }
    if (semctl(read_s, 0, SETVAL, 0) == -1) {
        fprintf(stderr, "read_s init error!\n");
        fflush(stderr);
        exit(-1);   
    }

    pthread_create(&tid_1, NULL, thread_routine_1, NULL);
    pthread_create(&tid_2, NULL, thread_routine_2, NULL);
    pthread_join(tid_1, NULL);
    pthread_join(tid_2, NULL);

    semctl(wrt_s, 0, IPC_RMID);
    semctl(read_s, 0, IPC_RMID);

    exit(0);
}