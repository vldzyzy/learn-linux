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
#define TIMEOUT 100
#define MAX_CLIENTS 10
#define max(a, b) ((a) > (b) ? (a) : (b))

// 广播消息给其他客户端

void broadcast(int sender_socket, char *message, int *client_sockets, int max_clients)
{
    for (int i = 0; i < max_clients; ++i)
    {
        if (client_sockets[i] != 0 && client_sockets[i] != sender_socket)
        {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
}

int main()
{
    int server_socket, new_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int client_sockets[MAX_CLIENTS] = {0};
    struct timeval timeout;
    time_t lastReceived = time(NULL);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    fd_set readfds;
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        int max_sd = server_socket;

        // 添加客户端socket到集合
        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            if (client_sockets[i] > 0)
            {
                FD_SET(client_sockets[i], &readfds);
                if (client_sockets[i] > max_sd)
                {
                    max_sd = client_sockets[i];
                }
            }
        }

        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select error");
            continue;
        }

        // 处理新连接
        if (FD_ISSET(server_socket, &readfds))
        {
            new_socket = accept(server_socket, NULL, NULL);
            if (new_socket < 0)
            {
                perror("accept failed");
                continue;
            }

            // 添加新客户端到数组
            for (int i = 0; i < MAX_CLIENTS; ++i)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    printf("new client connected, socket fd: %d\n", new_socket);
                    break;
                }
            }
        }

        // 处理客户端消息
        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &readfds))
            {
                int read_len = recv(client_sockets[i], buffer, BUFFER_SIZE, 0);

                if (read_len <= 0)
                {
                    printf("client connect break, socket fd: %d\n", client_sockets[i]);
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                }
                else
                {
                    buffer[read_len] = '\0';
                    printf("message received: %s\n", buffer);
                    broadcast(client_sockets[i], buffer, client_sockets, MAX_CLIENTS);
                }
            }
        }
    }

    return 0;
}