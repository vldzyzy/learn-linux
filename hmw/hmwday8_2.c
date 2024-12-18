#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define FIFO_1 "chat_FIFO_1"
#define FIFO_2 "chat_FIFO_2"
#define BUFFER_SIZE 1024
#define TIMEOUT 10

#define max(a, b) ((a) > (b) ? (a) : (b))

void chat_mode(int read_fd, int write_fd)
{
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    struct timeval timeout;
    time_t lastReceived = time(NULL);

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(read_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        int ready = select(max(STDERR_FILENO, read_fd) + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == -1)
        {
            fprintf(stderr, "Select error: %s\n", strerror(errno));
            return;
        }
        else if (ready == 0)
        {
            printf("Connection timeout!\n");
            return;
        }

        if (FD_ISSET(read_fd, &read_fds))
        {
            ssize_t bytes_read = read(read_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0)
            {
                printf("Other party closed the connection.\n");
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
                write(write_fd, buffer, strlen(buffer));
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
    if (mkfifo(FIFO_1, S_IRUSR | S_IWUSR) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "Failed to create FIFO_1: %s\n", strerror(errno));
        return 1;
    }

    if (mkfifo(FIFO_2, S_IRUSR | S_IWUSR) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "Failed to create FIFO_2: %s\n", strerror(errno));
        return 1;
    }

    int read_fd, write_fd;

    printf("choose a role: (1) user A or (2) user B\n");
    int choice;
    scanf("%d", &choice);
    getchar();

    if (choice == 1)
    {
        write_fd = open(FIFO_1, O_WRONLY);
        read_fd = open(FIFO_2, O_RDONLY);
        if (read_fd == -1 || write_fd == -1)
        {
            fprintf(stderr, "Failed to open FIFO: %s\n", strerror(errno));
            return 1;
        }
        printf("Connected as user A\n");

        chat_mode(read_fd, write_fd);
        close(write_fd);
        close(read_fd);
    }
    else if (choice == 2)
    {
        read_fd = open(FIFO_1, O_RDONLY);
        write_fd = open(FIFO_2, O_WRONLY);
        if (read_fd == -1 || write_fd == -1)
        {
            fprintf(stderr, "Failed to open FIFO: %s\n", strerror(errno));
            return 1;
        }

        printf("Connected as user B\n");
        chat_mode(read_fd, write_fd);
        close(write_fd);
        close(read_fd);
    }
    else
    {
        printf("Invalid choice. \n");
        return 1;
    }

    remove(FIFO_1);
    remove(FIFO_2);
    return 0;
}