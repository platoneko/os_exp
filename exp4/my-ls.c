#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define MAXLINE 256

void print_st_mode(ushort st_mode) {
    if (st_mode & )
}......................................




....














































































































int main(int argc, char *argv[]) {
    DIR *dir_ptr;
    struct dirent *dir_entry;
    struct stat *stat_buf;
    char filepath[MAXLINE];
    if (argc == 1) {  // .
        printf(".:\n");
        dir_ptr = opendir(".");
        while ((dir_entry = readdir(dir_ptr)) != NULL) {
            if (dir_entry->d_type == DT_DIR) {
                if (strcmp(dir_entry->d_name, ".") == 0 ||
                    strcmp(dir_entry->d_name, "..") == 0) {
                    continue;
                }
                
            } else if (dir_entry->d_type == DT_REG) {
                stat(dir_entry->d_name, stat_buf);
                printf("-")
            }
        }
    }
    
}