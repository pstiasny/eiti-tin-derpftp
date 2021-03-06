#include "types.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE   10240
#define MIN(x,y)  ( ((x) < (y)) ? (x) : (y) )

int send_reponse(int sock, int status, int value) {
    struct fs_response resp = {status, value};
    send(sock, &resp, sizeof(resp), 0);
    return 0;
}

int send_stat_reponse(int sock, int status, int value, struct stat *st) { 
    struct fs_stat_response resp;
    resp.base_response.status = status;
    resp.base_response.val = value;
    resp.stat.st_mode = st->st_mode; 
    resp.stat.st_size = st->st_size;
    resp.stat.st_atim = st->st_atim.tv_sec;
    resp.stat.st_mtim = st->st_mtim.tv_sec;
    resp.stat.st_ctim = st->st_ctim.tv_sec;
    resp.stat.st_blocks = st->st_blocks;
    resp.stat.st_blksize = st->st_blksize;   
    send(sock, &resp, sizeof(resp), 0);
    return 0;
}

int handle_connection(int sock, struct sockaddr_in *addr)
{
    int8_t cmd_type;
    struct fs_open_command cmd_open;
    struct fs_command cmd;
    int read_bytes, written_bytes, ret, file;
    char buffer[BUFSIZE], *bufp = 0;

    while (1 == recv(sock, &cmd_type, sizeof(cmd_type), MSG_PEEK)) {
        switch (cmd_type) {
        case FSMSG_OPEN:
            recv(sock, &cmd_open, sizeof(cmd_open), MSG_WAITALL);
            cmd_open.filename[255] = 0;
            printf("received FSMSG_OPEN, filename = %s\n", cmd_open.filename);

            file = open(cmd_open.filename, cmd_open.base_command.arg1, 0660);
            send_reponse(sock, errno, file);
        
            break;
        case FSMSG_WRITE:
            recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
            printf("received FSMSG_WRITE, fd = %i\n", cmd.fd);
            
            ret = 0;
            while (0 < (read_bytes = read(sock, buffer, MIN(BUFSIZE, cmd.arg1)))) {
                ret += read_bytes;
                cmd.arg1 -= read_bytes;
                write(cmd.fd, buffer, read_bytes);
            }
            send_reponse(sock, errno, ret);

            break;
        case FSMSG_READ:
            recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
            printf("received FSMSG_READ, fd = %i\n", cmd.fd);
		 
            read_bytes = read(cmd.fd, buffer, MIN(BUFSIZE, cmd.arg1));
            send_reponse(sock, errno, read_bytes);
            bufp = buffer;
            while (read_bytes) {
                written_bytes = write(sock, bufp, read_bytes);
                read_bytes -= written_bytes;
                bufp += written_bytes;
            }

            break;
        case FSMSG_LSEEK:
            recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
            printf("received FSMSG_LSEEK, fd = %i\n", cmd.fd);
		 
            ret = lseek(cmd.fd, cmd.arg1, cmd.arg2);
            send_reponse(sock, errno, ret);
            
            break;
        case FSMSG_CLOSE:
            recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
            printf("received FSMSG_CLOSE, fd = %i\n", cmd.fd);

            ret = close(cmd.fd);
            send_reponse(sock, errno, ret);
            
            break;
        case FSMSG_STAT:
            recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
            printf("received FSMSG_STAT, fd = %i\n", cmd.fd);

            struct stat sb;
            ret = fstat(cmd.fd, &sb);
            send_stat_reponse(sock, errno, ret, &sb);
            
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
    if (argc >= 2)
        addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (argc >= 3)
        addr.sin_port = htons(atoi(argv[2]));

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

