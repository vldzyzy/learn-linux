#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT 100
#define max(a, b) ((a) > (b) ? (a) : (b))

void chat_mode(int server_socket)
{
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    struct timeval timeout;
    time_t lastReceived = time(NULL);

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        int ready = select(max(server_socket, STDIN_FILENO) + 1, &read_fds, NULL, NULL, NULL);
        if (ready == -1)
        {
            fprintf(stderr, "select error: %s\n", strerror(errno));
            return;
        }
        else if (ready == 0)
        {
            printf("Connection timeout!\n");
            return;
        }

        if (FD_ISSET(server_socket, &read_fds))
        {
            ssize_t bytes_read = read(server_socket, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0)
            {
                printf("Server closed the connection.\n");
                return;
            }
            buffer[bytes_read] = '\0';
            printf("Received: %s", buffer);

            lastReceived = time(NULL);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL)
            {
                write(server_socket, buffer, strlen(buffer));
            }
        }

        // if (time(NULL) - lastReceived >= TIMEOUT)
        // {
        //     printf("Connection timeout!\n");
        //     return;
        // }
    }
}

int main()
{
    int socket_fd = 0;
    struct sockaddr_in serv_addr;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        printf("\nConnection Failed: inet_pton()\n");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed: connect()\n");
        return -1;
    }

    printf("Connected to the server\n");

    chat_mode(socket_fd);

    close(socket_fd);
    return 0;
}