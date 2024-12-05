#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#define FIFO_NAME "file_transfer_file"
#define BUFFER_SIZE 4096

int main(int argc, char *argv[]){
    if(argc == 2){
        if(mkfifo(FIFO_NAME, 0777) == -1 && errno != EEXIST){
            cerr << "Failed to create FIFO_SEND: " << strerror(errno) << endl;
            return 1;
        }
        int read_fd = open(argv[1], O_RDONLY);
        int write_fd = open(FIFO_NAME, O_WRONLY);

        int lenth = strlen(argv[1]);
        write(write_fd, &lenth, sizeof(int));
        write(write_fd, argv[1], lenth);

        char buffer[BUFFER_SIZE] = {0};
        ssize_t sret = read(read_fd, buffer, sizeof(buffer));
        write(write_fd, buffer, sret);

        close(write_fd);
        close(read_fd);
        remove(FIFO_NAME);
        return 0;
    }
    else if(argc == 1){
        if(mkfifo(FIFO_NAME, 0777) == -1 && errno != EEXIST){
            cerr << "Failed to create FIFO_SEND: " << strerror(errno) << endl;
            return 1;
        }
        int read_fd = open(FIFO_NAME, O_RDONLY);
        mkdir("./storage", 0777);
        char buffer[BUFFER_SIZE] = {0};
        int lenth;
        read(read_fd, &lenth, sizeof(int));
        read(read_fd, buffer, sizeof(buffer));
        char filepath[8192] = {0};
        sprintf(filepath, "%s/%s", "./storage", buffer);
        int write_fd = open(filepath, O_WRONLY|O_CREAT, 0777);
        memset(buffer, 0, sizeof(buffer));
        ssize_t sret = read(read_fd, buffer, sizeof(buffer));
        printf("sret = %ld\n", sret);
        write(write_fd, buffer, sret);
        close(write_fd);
        close(read_fd);
        remove(FIFO_NAME);
        return 0;
    }
    else return 1;
}