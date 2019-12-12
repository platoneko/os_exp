#include "global.h"

static int fd;

static void sig_handler(int sig) {
    printf("readbuf detect kill SIG\n");
    fflush(stdout);
    close(fd);
    exit(-1);
}

int main(int argc, char *argv[]) {
    int buf_shmid, head_p_shmid, tail_p_shmid, left_shmid;
    char *buf;
    
    int mtx;
    if ((mtx = semget(MTX_KEY, 1, 0)) == -1) {
        fprintf(stderr, "readbuf semget error!\n");
        fflush(stderr);
        exit(-1);
    }

    if ((buf_shmid = shmget(MEM_KEY, BUFSIZE, 0)) == -1) {
        fprintf(stderr,"readbuf buf_shmid semget error!\n");
        fflush(stderr);
        exit(-1);
    }
    if ((head_p_shmid = shmget(MEM_KEY+1, sizeof(int), 0)) == -1) {
        fprintf(stderr, "readbuf head_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);       
    }
    if ((tail_p_shmid = shmget(MEM_KEY+2, sizeof(int), 0)) == -1) {
        fprintf(stderr, "readbuf tail_p_shmid semget error!\n");
        fflush(stderr);
        exit(-1);    
    }
    if ((left_shmid = shmget(MEM_KEY+3, sizeof(int), 0)) == -1) {
        fprintf(stderr, "readbuf left_shmid semget error!\n");
        fflush(stderr);
        exit(-1);           
    }

    if ((int)(buf = shmat(buf_shmid, NULL, SHM_R)) == -1) {   // 将要读的位置
        fprintf(stderr, "readbuf buf shmat error!\n");
        fflush(stderr);
        exit(-1);         
    }
    int *head_p, *tail_p, *left;
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
    if ((int)(left = shmat(left_shmid, NULL, SHM_R | SHM_W)) == -1) {   // 剩余空间
        fprintf(stderr, "readbuf left shmat error!\n");
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

    int n; 
    while (1) {
        P(mtx, 0);
        if (*head_p != *tail_p) {
            printf("readbuf: %d\n", *head_p);
            char c = *(buf + *head_p);
            // fflush(stdout);
            ++*head_p;
            ++*left;
            if (*head_p == BUFSIZE) *head_p = 0;
            if (c == -1) {  // EOF
                printf("readbuf EOF\n");
                fflush(stdout);
                close(fd);
                V(mtx, 0);
                exit(0);
            }
            n = write(fd, &c, 1);
        }
        V(mtx, 0);
    } 
}