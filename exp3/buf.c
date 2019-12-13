#include "global.h"
#include "Block.h"

static int head_p_shmid, tail_p_shmid, block_shmid[LENGTH], read_n_shmid;
static int left, used;
static int wrt_p, read_p, status;

static void sigint_handler(int sig) {
    printf("main detect SIGINT\n");
    fflush(stdout);
    kill(wrt_p, SIGUSR1);
    kill(read_p, SIGUSR2);
    waitpid(wrt_p, &status, 0);
    waitpid(read_p, &status, 0);
    semctl(left, 0, IPC_RMID);
    semctl(used, 0, IPC_RMID);
    for (int i = 0; i < LENGTH; ++i) {
        shmctl(block_shmid[i], IPC_RMID, 0);
    }
    shmctl(head_p_shmid, IPC_RMID, 0);
    shmctl(tail_p_shmid, IPC_RMID, 0);
    shmctl(read_n_shmid, IPC_RMID, 0);
    exit(-1);
}


int main(int argc, char *argv[]) {
    if ((left = semget(MTX_KEY, 1, IPC_CREAT|0600)) == -1) {
        fprintf(stderr, "left create error!\n");
        fflush(stderr);
        exit(-1);
    }
    semctl(left, 0, SETVAL, LENGTH);
    if ((used = semget(MTX_KEY+1, 1, IPC_CREAT|0600)) == -1) {
        fprintf(stderr, "used create error!\n");
        fflush(stderr);
        exit(-1);
    }
    semctl(used, 0, SETVAL, 0);

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

    if ((read_n_shmid = shmget(MEM_KEY+LENGTH+2, sizeof(int),  IPC_CREAT | 0600)) == -1) {
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

    int *head_p, *tail_p, *read_n;
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
    if ((int)(read_n = shmat(read_n_shmid, NULL, SHM_R | SHM_W)) == -1) {   // writebuf最近一次读到的字节数
        fprintf(stderr, "main read_n shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    *head_p = 0;
    *tail_p = 0;
    *read_n = 0;

    if ((wrt_p = fork()) == 0) {
        execv("./writebuf", &argv[1]);  // 从input文件读并写入buf
    } else if ((read_p = fork()) == 0) {
        execv("./readbuf", &argv[2]);  // 从buf读并写入output文件
    }

    signal(SIGINT, sigint_handler);

    waitpid(wrt_p, &status, 0);
    waitpid(read_p, &status, 0);

    semctl(left, 0, IPC_RMID);
    semctl(used, 0, IPC_RMID);
    for (int i = 0; i < LENGTH; ++i) {
        shmctl(block_shmid[i], IPC_RMID, 0);
    }
    shmctl(head_p_shmid, IPC_RMID, 0);
    shmctl(tail_p_shmid, IPC_RMID, 0);
    shmctl(read_n_shmid, IPC_RMID, 0);
    exit(0);
}