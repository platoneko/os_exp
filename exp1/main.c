#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAXLINE 64

int pid[2]; 

void sigint_handler(int sig) {
    kill(pid[0], SIGUSR1);
    kill(pid[1], SIGUSR2);
}

void sigint_handler_1(int sig) {
    printf("Child Process 1 is Killed by Parent!\n");
    fflush(stdout);
    exit(0);
}

void sigint_handler_2(int sig) {
    printf("Child Process 2 is Killed by Parent!\n");
    fflush(stdout);
    exit(0);
}

int main() {
    int fd[2], status, x = 1;
    char wbuf[MAXLINE], rbuf[MAXLINE];

    pipe(fd);
    if ((pid[0] = fork()) == 0) {  // 子进程1
        signal(SIGINT, SIG_IGN);
        signal(SIGUSR1, sigint_handler_1);
        while (1) {
            sleep(1);
            sprintf(wbuf, "I send you %d times\n", x++);
            write(fd[1], wbuf, strlen(wbuf)+1);
        }
        exit(0);
    } else if ((pid[1] = fork()) == 0) {  // 子进程2
        signal(SIGINT, SIG_IGN);
        signal(SIGUSR2, sigint_handler_2);
        while (1) {
            read(fd[0], rbuf, MAXLINE);
            printf("%s", rbuf);
        }
        exit(0);
    }
    signal(SIGINT, sigint_handler);

    waitpid(pid[0], &status, 0);
    waitpid(pid[1], &status, 0);
    close(fd[0]);
    close(fd[1]);
    printf("Parent Process is Killed!\n");
    fflush(stdout);
    exit(0);
}