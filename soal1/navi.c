#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"

int sock;

void *recv_handler(void *arg)
{
    char buffer[1024];

    while (1)
    {
        int r = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (r <= 0) break;

        buffer[r] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main()
{
    struct sockaddr_in server;
    char msg[1024], name[50];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    printf("Connected to server\n");
    printf("Enter your name: ");

    fgets(name, sizeof(name), stdin);
    send(sock, name, strlen(name), 0);

    pthread_t tid;
    pthread_create(&tid, NULL, recv_handler, NULL);

    while (1)
    {
        fgets(msg, sizeof(msg), stdin);
        send(sock, msg, strlen(msg), 0);
    }

    return 0;
}
