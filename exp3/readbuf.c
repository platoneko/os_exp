#include "global.h"
#include "Block.h"

static int fd;

static void sig_handler(int sig) {
    printf("readbuf detect kill SIG\n");
    fflush(stdout);
    close(fd);
    exit(-1);
}

int main(int argc, char *argv[]) {
    int block_shmid[LENGTH], head_p_shmid, tail_p_shmid, read_n_shmid;
    
    int left, used;
    if ((left = semget(MTX_KEY, 1, 0)) == -1) {
        fprintf(stderr, "readbuf semget error!\n");
        fflush(stderr);
        exit(-1);
    }
    if ((used = semget(MTX_KEY+1, 1, 0)) == -1) {
        fprintf(stderr, "readbuf semget error!\n");
        fflush(stderr);
        exit(-1);
    }

    for (int i = 0; i < LENGTH; ++i) {
        if ((block_shmid[i] = shmget(MEM_KEY+i, sizeof(struct Block), 0)) == -1) {
            fprintf(stderr,"readbuf block_shmid semget error!\n");
            fflush(stderr);
            exit(-1);
        }
    }

    if ((head_p_shmid = shmget(MEM_KEY+LENGTH, sizeof(int), 0)) == -1) {
        fprintf(stderr, "readbuf head_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);       
    }
    if ((tail_p_shmid = shmget(MEM_KEY+LENGTH+1, sizeof(int), 0)) == -1) {
        fprintf(stderr, "readbuf tail_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);    
    }
    if ((read_n_shmid = shmget(MEM_KEY+LENGTH+2, sizeof(int), 0)) == -1) {
        fprintf(stderr, "readbuf read_n_shmid semget error!\n");
        fflush(stderr);
        exit(-1);           
    }

    int *head_p, *tail_p, *read_n;
    if ((int)(head_p = shmat(head_p_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 将要读的位置
        fprintf(stderr, "readbuf head_p shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(tail_p = shmat(tail_p_shmid, NULL, SHM_R)) == -1) {   // 将要写的位置
        fprintf(stderr, "readbuf tail_p shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    if ((int)(read_n = shmat(read_n_shmid, NULL, SHM_R)) == -1) {   // writebuf最近一次读到的字节数
        fprintf(stderr, "readbuf read_n shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }

    if ((fd = open(argv[0], O_WRONLY)) == -1) {
        fprintf(stderr, "open %s error!\n", argv[0]);
        fflush(stderr);
        exit(-1);     
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGUSR2, sig_handler);

    struct Block *block;
    while (1) {
        P(used, 0);
        if (*read_n != BLOCKSIZE && (*head_p == *tail_p-1 || *head_p == *tail_p-1+LENGTH)) {  // EOF
            printf("readbuf EOF\n");
            if (*read_n) { // EOF之前还有数据
                block = shmat(block_shmid[*head_p], NULL, SHM_R);
                V(left, 0);
                write(fd, block->data, *read_n);
                printf("readbuf: %d\n", *head_p);
            }
            fflush(stdout);
            close(fd);
            exit(0);
        }
        block = shmat(block_shmid[*head_p], NULL, SHM_R);
        write(fd, block->data, BLOCKSIZE);
        printf("readbuf: %d\n", *head_p);
        fflush(stdout);
        ++*head_p;
        if (*head_p == LENGTH) *head_p = 0;
        V(left, 0);
    } 
}