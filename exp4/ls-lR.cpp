#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAXLINE 1024
#define NONE                 "\e[0m"
#define BLACK                "\e[0;30m"
#define L_BLACK              "\e[1;30m"
#define RED                  "\e[0;31m"
#define L_RED                "\e[1;31m"
#define GREEN                "\e[0;32m"
#define L_GREEN              "\e[1;32m"
#define BROWN                "\e[0;33m"
#define YELLOW               "\e[1;33m"
#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
#define PURPLE               "\e[0;35m"
#define L_PURPLE             "\e[1;35m"
#define CYAN                 "\e[0;36m"
#define L_CYAN               "\e[1;36m"
#define GRAY                 "\e[0;37m"
#define WHITE                "\e[1;37m"

#define BOLD                 "\e[1m"
#define UNDERLINE            "\e[4m"
#define BLINK                "\e[5m"
#define REVERSE              "\e[7m"
#define HIDE                 "\e[8m"
#define CLEAR                "\e[2J"
#define CLRLINE              "\r\e[K"
#define ERROR_C              "\e[1;31;40m"


struct FileInfo {
    std::string name;
    struct stat file_stat;
    u_char d_type;
};

void print_dir(DIR *dir_ptr);
void print_file(const struct FileInfo &file_info);

void print_file(const struct FileInfo &file_info) {
    char mode[11] = {0};
    char *time_ptr;
    if (file_info.d_type == DT_DIR) mode[0] = 'd';
    else if (file_info.d_type == DT_LNK) mode[0] = 'l';
    else mode[0] = '-';
    mode[1] = ((file_info.file_stat.st_mode & S_IRUSR)? 'r':'-');
    mode[2] = ((file_info.file_stat.st_mode & S_IWUSR)? 'w':'-');
    mode[3] = ((file_info.file_stat.st_mode & S_IXUSR)? 'x':'-');
    mode[4] = ((file_info.file_stat.st_mode & S_IRGRP)? 'r':'-');
    mode[5] = ((file_info.file_stat.st_mode & S_IWGRP)? 'w':'-');
    mode[6] = ((file_info.file_stat.st_mode & S_IXGRP)? 'x':'-');
    mode[7] = ((file_info.file_stat.st_mode & S_IROTH)? 'r':'-');
    mode[8] = ((file_info.file_stat.st_mode & S_IWOTH)? 'w':'-');
    mode[9] = ((file_info.file_stat.st_mode & S_IXOTH)? 'x':'-');
    std::cout << mode << "\t";
    std::cout << file_info.file_stat.st_nlink << "\t";
    std::cout << getpwuid(file_info.file_stat.st_uid)->pw_name << "\t";
    std::cout << getgrgid(file_info.file_stat.st_gid)->gr_name << "\t";
    std::cout << file_info.file_stat.st_size << "\t";
    time_ptr = ctime(&file_info.file_stat.st_mtim.tv_sec);
    time_ptr[strlen(time_ptr)-1] = 0;
    std::cout << time_ptr << "\t";
    if (file_info.d_type == DT_DIR) {
        std::cout << L_BLUE;
        std::cout << file_info.name;
        std::cout << NONE;
    } else if (file_info.d_type == DT_REG){
        if (mode[3] == 'x' || mode[6] == 'x' or mode[9] == 'x') {
            std::cout << L_GREEN;
            std::cout << file_info.name;
            std::cout << NONE;
        } else {
            std::cout << file_info.name;
        }
    } else if (file_info.d_type == DT_LNK) {
        char filename[MAXLINE] = {0};
        readlink(file_info.name.c_str(), filename, MAXLINE);
        struct stat stat_buf;
        if (stat(filename, &stat_buf) < 0) {
            std::cout << ERROR_C << file_info.name << NONE << " -> " << ERROR_C << filename << NONE;
        } else {
            std::cout << L_CYAN;
            std::cout << file_info.name;
            std::cout << NONE;
            std::cout << " -> ";
            if (stat_buf.st_mode & S_IFDIR) {
                std::cout << L_BLUE;
                std::cout << filename;
                std::cout << NONE;
            } else if (stat_buf.st_mode & S_IFREG) {
                if (stat_buf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
                    std::cout << L_GREEN;
                    std::cout << filename;
                    std::cout << NONE;
                } else {
                    std::cout << filename;
                }
            } else {
                std::cout << filename;
            }
        }
    }
    std::cout << "\n";
}

void print_dir(DIR *dir_ptr, std::string path) {
    struct dirent *dir_entry;
    struct stat stat_buf;
    std::vector<std::string> dir_vec;
    std::vector<struct FileInfo> file_vec;
    uint total = 0;
    while ((dir_entry = readdir(dir_ptr)) != NULL) {
        struct FileInfo file_info;
        if (dir_entry->d_name[0] == '.') continue;
        if (dir_entry->d_type == DT_DIR) {
            file_info.name = dir_entry->d_name;
            stat(dir_entry->d_name, &stat_buf);
            file_info.file_stat = stat_buf;
            file_info.d_type = dir_entry->d_type;
            file_vec.push_back(file_info);
            total += (file_info.file_stat.st_blocks >> 1);
        } else if (dir_entry->d_type == DT_LNK) {
            file_info.name = dir_entry->d_name;
            lstat(dir_entry->d_name, &stat_buf);
            file_info.file_stat = stat_buf;
            file_info.d_type = dir_entry->d_type;
            file_vec.push_back(file_info);
        } else if (dir_entry->d_type == DT_REG) {
            file_info.name = dir_entry->d_name;
            stat(dir_entry->d_name, &stat_buf);
            file_info.file_stat = stat_buf;
            file_info.d_type = dir_entry->d_type;
            file_vec.push_back(file_info);
            total += (file_info.file_stat.st_blocks >> 1);
        }
    }
    sort(file_vec.begin(), file_vec.end(), [](const struct FileInfo &a, const struct FileInfo &b){return a.name < b.name;});

    std::cout << path << ":\n" << "total " << total << std::endl;
    for (const auto &file_info: file_vec) {
        print_file(file_info);
        if (file_info.d_type == DT_DIR) {
            dir_vec.push_back(file_info.name);
        }
    }
    std::cout << std::endl;
    
    for (const auto &dir_name: dir_vec) {
        DIR *subdir_ptr = opendir(dir_name.c_str());
        chdir(dir_name.c_str());
        std::string subpath = path + "/" + dir_name;
        print_dir(subdir_ptr, subpath);
        closedir(subdir_ptr);
        chdir("..");
    }
}

int main(int argc, char *argv[]) {

    DIR *dir_ptr;

    if (argc == 1) {  // .
        dir_ptr = opendir(".");
        print_dir(dir_ptr, ".");
        closedir(dir_ptr); 
    } else {
        struct stat stat_buf;
        char cwd_buf[MAXLINE];
        getcwd(cwd_buf, MAXLINE);
        for (int i=1; i<argc; ++i) {
            if (lstat(argv[i], &stat_buf) < 0) {
                std::cerr << "cannot access '" << argv[i] << "': No such file or directory" << std::endl;
                exit(-1);
            }
            if (stat_buf.st_mode & S_IFDIR) {
                dir_ptr = opendir(argv[i]);
                chdir(argv[i]);
                print_dir(dir_ptr, argv[i]);
                closedir(dir_ptr);
                chdir(cwd_buf);
            } else {
                struct FileInfo file_info;
                file_info.file_stat = stat_buf;
                file_info.name = argv[i];
                if ((stat_buf.st_mode & S_IFLNK) == S_IFLNK) {
                    file_info.d_type = DT_LNK;
                    print_file(file_info);
                    std::cout << std::endl;                   
                } else if (stat_buf.st_mode & S_IFREG) {
                    file_info.d_type = DT_REG;
                    print_file(file_info);
                    std::cout << std::endl;
                }
            }
        }
    }
    exit(0);
}