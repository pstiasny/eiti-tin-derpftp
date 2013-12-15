#include "types.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int send_reponse(int sock, int status, int value) {
    struct fs_response resp = {status, value};
    write(sock, &resp, sizeof(resp)); /* TODO: send all */
    return 0;
}

int handle_connection(int sock, struct sockaddr_in *addr)
{
    int8_t cmd_type;
    struct fs_open_command cmd;
    int read_bytes, i, file;
    while (1 == recv(sock, &cmd_type, sizeof(cmd_type), MSG_PEEK)) {
        switch (cmd_type) {
        case FSMSG_OPEN:
            recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
            cmd.filename[255] = 0;
            printf("received FSMSG_OPEN, filename = %s\n", cmd.filename);

            file = open(cmd.filename, cmd.base_command.arg1);
            send_reponse(sock, 0, file);

            break;
        default:
            printf("unknown msg type %d\n", cmd_type);
            return 1;
        }
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

