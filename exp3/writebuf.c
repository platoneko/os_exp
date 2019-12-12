#include "global.h"
#include "Block.h"

static int fd;

static void sig_handler(int sig) {
    printf("writebuf detect kill SIG\n");
    fflush(stdout);
    close(fd);
    exit(-1);
}

int main(int argc, char *argv[]) {
    int block_shmid[LENGTH], head_p_shmid, tail_p_shmid, left_shmid, read_n_shmid;
    
    int mtx;
    if ((mtx = semget(MTX_KEY, 1, 0)) == -1) {
        fprintf(stderr, "writebuf semget error!\n");
        fflush(stderr);
        exit(-1);
    }

    for (int i = 0; i < LENGTH; ++i) {
        if ((block_shmid[i] = shmget(MEM_KEY+i, sizeof(struct Block), 0)) == -1) {
            fprintf(stderr,"writebuf block_shmid semget error!\n");
            fflush(stderr);
            exit(-1);
        }
    }

    if ((head_p_shmid = shmget(MEM_KEY+LENGTH, sizeof(int), 0)) == -1) {
        fprintf(stderr, "writebuf head_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);       
    }
    if ((tail_p_shmid = shmget(MEM_KEY+LENGTH+1, sizeof(int), 0)) == -1) {
        fprintf(stderr, "writebuf tail_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);    
    }
    if ((left_shmid = shmget(MEM_KEY+LENGTH+2, sizeof(int), 0)) == -1) {
        fprintf(stderr, "writebuf left_shmid semget error!\n");
        fflush(stderr);
        exit(-1);           
    }
    if ((read_n_shmid = shmget(MEM_KEY+LENGTH+3, sizeof(int), 0)) == -1) {
        fprintf(stderr, "writebuf read_n_shmid semget error!\n");
        fflush(stderr);
        exit(-1);           
    }

    int *head_p, *tail_p, *left, *read_n;
    if ((int)(head_p = shmat(head_p_shmid, NULL, SHM_R)) == -1) {   // 将要读的位置
        fprintf(stderr, "writebuf head_p shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(tail_p = shmat(tail_p_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 将要写的位置
        fprintf(stderr, "writebuf tail_p shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(left = shmat(left_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 剩余空间
        fprintf(stderr, "writebuf left shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(read_n = shmat(read_n_shmid, NULL, SHM_R | SHM_W)) == -1) {   // writebuf已经遇到EOF
        fprintf(stderr, "writebuf read_n shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }

    if ((fd = open(argv[0], O_RDONLY)) == -1) {
        fprintf(stderr, "open %s error!\n", argv[0]);
        fflush(stderr);
        exit(-1);     
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, sig_handler);

    struct Block *block;
    while (1) {
        P(mtx, 0);
        if (*left) {  // 还有空余空间
            printf("writebuf: %d\n", *tail_p);
            block = shmat(block_shmid[*tail_p], NULL, SHM_W);
            *read_n = read(fd, block->data, BLOCKSIZE);
            --*left;
            ++*tail_p;
            if (*tail_p == LENGTH) *tail_p = 0;
            if (*read_n != BLOCKSIZE) {
                printf("writebuf EOF\n");
                fflush(stdout);
                close(fd);
                V(mtx, 0);
                exit(0);
            }
        }
        V(mtx, 0);
    } 
}