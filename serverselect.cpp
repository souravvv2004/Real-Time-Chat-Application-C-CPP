#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 256

void error(const char* msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int portno = atoi(argv[1]);

    // 1. create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("socket failed");

    // allow quick restart of server on same port
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // 2. setup address struct
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // 3. bind
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("bind failed");

    // 4. listen
    if (listen(sockfd, 5) < 0)
        error("listen failed");

    printf("Server listening on port %d...\n", portno);

    // array to track connected client file descriptors
    int client_fds[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_fds[i] = -1;

    fd_set readfd;
    char buffer[BUF_SIZE];

    for (;;)
    {
        // ---- build the fd_set fresh every iteration ----
        FD_ZERO(&readfd);
        FD_SET(sockfd, &readfd);
        int maxfd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] != -1)
            {
                FD_SET(client_fds[i], &readfd);
                if (client_fds[i] > maxfd)
                    maxfd = client_fds[i];
            }
        }

        int activity = select(maxfd + 1, &readfd, NULL, NULL, NULL);
        if (activity < 0)
            error("select failed");

        // ---- case 1: new client trying to connect ----
        if (FD_ISSET(sockfd, &readfd))
        {
            struct sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);

            int newfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
            if (newfd < 0)
            {
                perror("accept failed");
            }
            else
            {
                int placed = 0;
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (client_fds[i] == -1)
                    {
                        client_fds[i] = newfd;
                        printf("New client connected (fd=%d), slot %d\n", newfd, i);
                        placed = 1;
                        break;
                    }
                }
                if (!placed)
                {
                    printf("Max clients reached, rejecting new connection\n");
                    close(newfd);
                }
            }
        }

        // ---- case 2: existing client sent data ----
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] == -1)
                continue;

            if (FD_ISSET(client_fds[i], &readfd))
            {
                bzero(buffer, sizeof(buffer));
                int n = recv(client_fds[i], buffer, sizeof(buffer) - 1, 0);

                if (n <= 0)
                {
                    // client disconnected or error
                    printf("Client (fd=%d) disconnected\n", client_fds[i]);
                    close(client_fds[i]);
                    client_fds[i] = -1;
                }
                else
                {
                    printf("Client (fd=%d): %s", client_fds[i], buffer);

                    // broadcast message to all OTHER connected clients
                    for (int j = 0; j < MAX_CLIENTS; j++)
                    {
                        if (client_fds[j] != -1 && client_fds[j] != client_fds[i])
                        {
                            send(client_fds[j], buffer, n, 0);
                        }
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
