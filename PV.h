#ifndef PV_H
#define PV_H 
#include <sys/sem.h>

void P(int semid, int sem_num);
void V(int semid, int sem_num);

#endif