/* 实现即时聊天，如果对方连续10s没有发送任何消息则断开连接（注意，即使本方在10s内从标准输入当中输入数据也不行）*/
#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/select.h>
#include <cstdio>

using namespace std;

#define FIFO_SEND "chat_fifo_send"
#define FIFO_RECEIVE "chat_fifo_receive"
#define BUFFER_SIZE 1024
#define TIMEOUT 10 // 超时时间10秒

void chat_mode(int read_fd, int write_fd){
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    struct timeval timeout;
    time_t lastReceived = time(NULL);
    
    while(true){
        FD_ZERO(&read_fds);
        FD_SET(read_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        int ready = select(max(STDERR_FILENO, read_fd) + 1, &read_fds, NULL, NULL, &timeout);
        if(ready == -1){
            cerr << "Select error: " << strerror(errno) << endl;
            return; 
        } else if(ready == 0){
            cout << "Connection timeout!" << endl;
            return;
        }
        // 处理接收到的数据
        if(FD_ISSET(read_fd, &read_fds)){
            ssize_t bytes_read = read(read_fd, buffer, sizeof(buffer) - 1);
            if(bytes_read <= 0){
                cout << "Other party closed the connection." << endl;
                return;
            }
            buffer[bytes_read] = '\0';
            cout << "Received: " << buffer;

            lastReceived = time(NULL);
        }
        // 处理用户输入的数据
        if(FD_ISSET(STDIN_FILENO, &read_fds)){
            if(fgets(buffer, sizeof(buffer), stdin) != NULL){
                write(write_fd, buffer, strlen(buffer));
            }
        }

        if(time(NULL) - lastReceived >= TIMEOUT){
            cout << "Connection timeout!" << endl;
            return;
        }
    }
}

int main(){
    if(mkfifo(FIFO_SEND, S_IRUSR | S_IWUSR) == -1 && errno != EEXIST){
        cerr << "Failed to create FIFO_SEND: " << strerror(errno) << endl;
        return 1;
    }

    if(mkfifo(FIFO_RECEIVE, S_IRUSR | S_IWUSR) == -1 && errno != EEXIST){
        cerr << "Failed to create FIFO_RECEIVE: " << strerror(errno) << endl;
        return 1;
    }

    int read_fd, write_fd;

    cout << "Choose a role: (1) User A or (2) User B" << endl;
    int choice;
    cin >> choice;
    cin.ignore();

    if(choice == 1){
        write_fd = open(FIFO_SEND, O_WRONLY);
        read_fd = open(FIFO_RECEIVE, O_RDONLY);
        if(read_fd == -1 || write_fd == -1){
            cerr << "Failed to open FIFO:" << strerror(errno) << endl;
            return 1;
        }

        cout << "Connected as User A" << endl;
    
        chat_mode(read_fd, write_fd);

        close(read_fd);
        close(write_fd);
    }
    else if(choice == 2){
        read_fd = open(FIFO_SEND, O_RDONLY);
        write_fd = open(FIFO_RECEIVE, O_WRONLY);
        if(read_fd == -1 ||write_fd == -1){
            cerr << "Failed to open FIFO:" << strerror(errno) << endl;
            return 1;
        }

        cout << "Connected as User B" << endl;

        chat_mode(read_fd, write_fd);

        close(write_fd);
        close(read_fd);
    }
    else{
        cout << "Invalid choice." << endl;
        return 1;
    }

    remove(FIFO_SEND);
    remove(FIFO_RECEIVE);
    return 0;
}