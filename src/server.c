#include "types.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int handle_connection(int sock, struct sockaddr_in *addr)
{
    int read_bytes, i;
    char cmd[16], *cmdp;
    while (0 < (read_bytes = read(sock, cmd, sizeof(cmd)))) {
        cmdp = cmd;
        while(read_bytes--)
            printf("%x ", *cmdp++);
        printf("\n");
    }
    printf("subprocess exiting\n");
    return 0;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int listener_sock = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(1337);

    int yes=1;
    if (-1 == setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
        perror("setsockopt");
        return 1;
    }
    if (-1 == bind(listener_sock, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("bind");
        return 1;
    }
    if (-1 == listen(listener_sock, 128)) {
        perror("listen");
        return 1;
    }

    while (1) {
        int handler_pid;
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int connection_sock;

        connection_sock = accept(
            listener_sock, (struct sockaddr*)&client_addr, &len);
        if (connection_sock < 0)
            return 1;

        handler_pid = fork();
        if (handler_pid == 0) {
            close(listener_sock);
            return handle_connection(connection_sock, &client_addr);
        } else {
            printf("connection from %s spawned process %d\n",
                inet_ntoa(client_addr.sin_addr), handler_pid);
            close(connection_sock);
        }
    }

    return 0;
}

