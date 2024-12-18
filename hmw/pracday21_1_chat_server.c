#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT 10
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

        int ready = select(max(server_socket, STDIN_FILENO) + 1, &read_fds, NULL, NULL, &timeout);
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
                printf("client closed the connection.\n");
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

        if (time(NULL) - lastReceived >= TIMEOUT)
        {
            printf("Connection timeout!\n");
            return;
        }
    }
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Connected to a client\n");

    chat_mode(new_socket);

    close(new_socket);
    close(server_fd);

    return 0;
}