#ifndef GLOBAL_H
#define GLOBAL_H

#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<fcntl.h>
#include <sys/wait.h>
#include "../PV.h"

#define BUFSIZE 1024
#define BLOCKSIZE 128
#define LENGTH 8
#define MEM_KEY 33333
#define MTX_KEY 3333
#endif