#include "PV.h"
#include <stdio.h>
#include <stdlib.h>

void P(int semid, int sem_num) {
    struct sembuf sem;
    sem.sem_num = sem_num;
    sem.sem_op = -1;
    sem.sem_flg = SEM_UNDO;
    if (semop(semid, &sem, 1) == -1) {
        fprintf(stderr, "P failed\n");
        fflush(stderr);
        exit(-1);
    }
}

void V(int semid, int sem_num) {
    struct sembuf sem;
    sem.sem_num = sem_num;
    sem.sem_op = 1;
    sem.sem_flg = SEM_UNDO;
    if (semop(semid, &sem, 1) == -1) {
        fprintf(stderr, "V failed\n");
        fflush(stderr);
        exit(-1);
    }
}