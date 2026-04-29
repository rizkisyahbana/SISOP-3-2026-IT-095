#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <stdlib.h>
#include "protocol.h"

int clients[MAX];
char usernames[MAX][50];
time_t start_time;

void log_event(const char *type, const char *msg)
{
    FILE *fp = fopen("history.log", "a");
    if (!fp) return;


    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);


    fprintf(fp, "[%s] [%s] [%s]\n", timebuf, type, msg);
    fclose(fp);
}

int name_exists(char *name)
{
    for (int i = 0; i < MAX; i++)
        if (strcmp(usernames[i], name) == 0)
            return 1;
    return 0;

}

void broadcast(char *msg, int sender)
{
    for (int i = 0; i < MAX; i++)
    {
        if (clients[i] && clients[i] != sender)

            send(clients[i], msg, strlen(msg), 0);
    }
}

int main()
{
    int server_fd, client_sock, activity;
    struct sockaddr_in addr;

    fd_set readfds;
    char buffer[1024];

    start_time = time(NULL);


    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

    listen(server_fd, 10);

    printf("Server running...\n");
    log_event("System", "SERVER ONLINE");

    while (1)

    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        int max = server_fd;

        for (int i = 0; i < MAX; i++)

        {
            if (clients[i])
            {
                FD_SET(clients[i], &readfds);
                if (clients[i] > max) max = clients[i];
            }

        }

        activity = select(max + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds))
        {

            client_sock = accept(server_fd, NULL, NULL);

            for (int i = 0; i < MAX; i++)
            {
                if (!clients[i])
                {

                    clients[i] = client_sock;
                    break;
                }
            }


            printf("Client connected\n");
        }

        for (int i = 0; i < MAX; i++)

        {
            int sd = clients[i];

            if (sd && FD_ISSET(sd, &readfds))
            {

                int r = recv(sd, buffer, sizeof(buffer)-1, 0);

                if (r <= 0)
                {
                    close(sd);

                    clients[i] = 0;
                    usernames[i][0] = '\0';
                    continue;
                }

                buffer[r] = '\0';

                buffer[strcspn(buffer, "\n")] = 0;

                if (strlen(buffer) > 50)
                    buffer[50] = '\0';


                if (strlen(usernames[i]) == 0)
                {
                    if (name_exists(buffer))
                    {
                        char msg[] = "[System] Name already exists\n";

                        send(sd, msg, strlen(msg), 0);
                    }
                    else
                    {
                        strcpy(usernames[i], buffer);


                        char msg[200];
                        snprintf(msg, sizeof(msg),
                                 "--- Welcome to The Wired, %.50s ---\n",
                                 buffer);


                        send(sd, msg, strlen(msg), 0);

                        char logmsg[200];
                        snprintf(logmsg, sizeof(logmsg),
                                 "User '%.50s' connected",
                                 buffer);


                        log_event("System", logmsg);
                    }
                }
                else
                {

                    if (strncmp(buffer, "admin:", 6) == 0)
                    {
                        if (strcmp(usernames[i], "The Knights") == 0)
                        {
                            char cmd = buffer[6];


                            if (cmd == '1')
                            {
                                int count = 0;
                                for (int j = 0; j < MAX; j++)
                                    if (clients[j]) count++;


                                char msg[100];
                                snprintf(msg, sizeof(msg),
                                         "[Admin] Active Users: %d\n", count);
                                send(sd, msg, strlen(msg), 0);

                                log_event("Admin", "RPC_GET_USERS");
                            }
                            else if (cmd == '2')
                            {
                                time_t now = time(NULL);
                                int uptime = (int)difftime(now, start_time);

                                char msg[100];
                                snprintf(msg, sizeof(msg),
                                         "[Admin] Uptime: %d seconds\n", uptime);
                                send(sd, msg, strlen(msg), 0);

                                log_event("Admin", "RPC_GET_UPTIME");
                            }
                            else if (cmd == '3')
                            {
                                log_event("System", "SHUTDOWN");
                                printf("Server shutting down...\n");
                                close(server_fd);
                                exit(0);
                            }

                        }
                        continue;
                    }

                    char msg[1200];
                    snprintf(msg, sizeof(msg),
                             "[%s]: %s\n",
                             usernames[i], buffer);

                    printf("%s", msg);
                    log_event("User", msg);

                    broadcast(msg, sd);
                }

            }
        }
    }

    return 0;
}
