#include "global.h"
#include "Block.h"

static int head_p_shmid, tail_p_shmid, left_shmid, block_shmid[LENGTH], read_n_shmid;
static int mtx;
static int wrt_p, read_p, status;

static void sigint_handler(int sig) {
    printf("main detect SIGINT\n");
    fflush(stdout);
    kill(wrt_p, SIGUSR1);
    kill(read_p, SIGUSR2);
    waitpid(wrt_p, &status, 0);
    waitpid(read_p, &status, 0);
    semctl(mtx, 0, IPC_RMID);
    for (int i = 0; i < LENGTH; ++i) {
        shmctl(block_shmid[i], IPC_RMID, 0);
    }
    shmctl(head_p_shmid, IPC_RMID, 0);
    shmctl(tail_p_shmid, IPC_RMID, 0);
    shmctl(left_shmid, IPC_RMID, 0);
    shmctl(read_n_shmid, IPC_RMID, 0);
    exit(-1);
}


int main(int argc, char *argv[]) {
    if ((mtx = semget(MTX_KEY, 1, IPC_CREAT|0600)) == -1) {
        fprintf(stderr, "mtx create error!\n");
        fflush(stderr);
        exit(-1);
    }
    semctl(mtx, 0, SETVAL, 1);

    if ((head_p_shmid = shmget(MEM_KEY+LENGTH, sizeof(int),  IPC_CREAT | 0600)) == -1) {
        fprintf(stderr, "main head_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);       
    }
    if ((tail_p_shmid = shmget(MEM_KEY+LENGTH+1, sizeof(int),  IPC_CREAT | 0600)) == -1) {
        fprintf(stderr, "main tail_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);    
    }
    if ((left_shmid = shmget(MEM_KEY+LENGTH+2, sizeof(int),  IPC_CREAT | 0600)) == -1) {
        fprintf(stderr, "main left_shmid semget error!\n");
        fflush(stderr);
        exit(-1);           
    }
    if ((read_n_shmid = shmget(MEM_KEY+LENGTH+3, sizeof(int),  IPC_CREAT | 0600)) == -1) {
        fprintf(stderr, "main read_n_shmid semget error!\n");
        fflush(stderr);
        exit(-1);           
    }

    for (int i = 0; i < LENGTH; ++i) {
        if ((block_shmid[i] = shmget(MEM_KEY+i, sizeof(struct Block),  IPC_CREAT | 0600)) == -1) {
            fprintf(stderr,"main block_shmid semget error!\n");
            fflush(stderr);
            exit(-1);
        }
    }

    int *left, *head_p, *tail_p, *read_n;
    if ((int)(head_p = shmat(head_p_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 将要读的位置
        fprintf(stderr, "main head_p shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(tail_p = shmat(tail_p_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 将要写的位置
        fprintf(stderr, "main tail_p shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(left = shmat(left_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 剩余空间
        fprintf(stderr, "main left shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(read_n = shmat(read_n_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 剩余空间
        fprintf(stderr, "main read_n shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    *head_p = 0;
    *tail_p = 0;
    *left = LENGTH;
    *read_n = 0;

    if ((wrt_p = fork()) == 0) {
        execv("./writebuf", &argv[1]);  // 从input文件读并写入buf
    } else if ((read_p = fork()) == 0) {
        execv("./readbuf", &argv[2]);  // 从buf读并写入output文件
    }

    signal(SIGINT, sigint_handler);

    waitpid(wrt_p, &status, 0);
    waitpid(read_p, &status, 0);

    semctl(mtx, 0, IPC_RMID);
    for (int i = 0; i < LENGTH; ++i) {
        shmctl(block_shmid[i], IPC_RMID, 0);
    }
    shmctl(head_p_shmid, IPC_RMID, 0);
    shmctl(tail_p_shmid, IPC_RMID, 0);
    shmctl(left_shmid, IPC_RMID, 0);
    shmctl(read_n_shmid, IPC_RMID, 0);
    exit(0);
}