#include "api.h"
#include "types.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX_SERVERS 32
#define INV_HANDLE(h) ((h < 0) || (h >= MAX_SERVERS) || !servers[h].in_use)
static struct serverd {
    int in_use;
    int sock;
} servers[MAX_SERVERS];

int get_free_handle()
{
    int i;

    for (i = 0; i < MAX_SERVERS; ++i)
        if (!servers[i].in_use)
            return i;
    return -1;
}

int write_command(int sock, enum FSMSG_TYPE type, int32_t fd, int32_t arg1, int32_t arg2)
{
    struct fs_command cmd = { type, fd, arg1, arg2 };
    void *cmdp = &cmd;
    size_t sent, length = sizeof(cmd);

    while (0 != (sent = send(sock, cmdp, length, 0))) {
        if (sent == -1)
            return -1;

        length -= sent;
        cmdp += sent;
    }

    return 0;
}

int read_response(int sock, struct fs_response *buf)
{
    size_t len = sizeof(struct fs_response);
    void *bufp = buf;
    ssize_t got;
    while (len) {
        got = read(sock, bufp, len);
        if (got == -1)
            return -1;
        len -= got;
        bufp += got;
    }
    return 0;
}

/* on success returns response value
 * on failure returns FSE_FAIL and sets errno
 */
int process_response(struct fs_response *res)
{
    if (res->status) {
        errno = res->status;
        return FSE_FAIL;
    } else {
        return res->val;
    }
}

int fs_open_server(const char *server_addr)
{
    int server_handle, port = 1337;
    struct serverd *sd;
    struct sockaddr_in addr;
    struct hostent *hp;
    char *port_part;

    if (-1 == (server_handle = get_free_handle()))
        return FSE_CON_LIMIT;
    sd = &servers[server_handle]; 

    /* if server_addr is of the form "host:port", parse the port part */
    if (0 != (port_part = strchr(server_addr, ':'))) {
        port = atoi(port_part + 1);
        *port_part = 0;
    }

    /* resolve the name */
    hp = gethostbyname(server_addr);
    if (hp == 0)
        return FSE_UNKNOWN_HOST;
    memcpy((char*)&addr.sin_addr, (char*)hp->h_addr, hp->h_length);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    /* create a socket */
    sd->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sd->sock < 0)
        return FSE_CON_ERROR;

    /* connect */
    if (-1 == connect(sd->sock, (struct sockaddr*)&addr, sizeof(addr)))
        return FSE_CON_ERROR;

    sd->in_use = 1;
    return FSE_OK;
}

int fs_close_server(int server_handle)
{
    if (INV_HANDLE(server_handle))
        return FSE_INVALID_HANDLE;

    servers[server_handle].in_use = 0;
    return close(servers[server_handle].sock);
}

int fs_open(int server_handle, const char *name, int flags)
{
    struct fs_open_command cmd;
    struct fs_response res;

    if (INV_HANDLE(server_handle))
        return FSE_INVALID_HANDLE;

    memset(&cmd, 0, sizeof(cmd));
    cmd.base_command.type = FSMSG_OPEN;
    cmd.base_command.arg1 = flags;
    strncpy(cmd.filename, name, 256);
    if (sizeof(cmd) != write(servers[server_handle].sock, &cmd, sizeof(cmd)))
        return FSE_CON_ERROR;

    if (-1 == read_response(servers[server_handle].sock, &res))
        return FSE_CON_ERROR;

    return process_response(&res);
}

int fs_write(int server_handle, int fd, void *buf, size_t len)
{
    struct fs_response res;
    size_t sent = 0;
    int sock;

    if (INV_HANDLE(server_handle))
        return FSE_INVALID_HANDLE;
    sock = servers[server_handle].sock;

    if (-1 == write_command(sock, FSMSG_WRITE, fd, len, 0))
        return FSE_CON_ERROR;

    for (; len > 0; len -= sent) {
        sent = send(sock, buf, len, 0);
        if (sent == -1)
            return FSE_CON_ERROR;

        buf += sent;
    }

    if (-1 == read_response(sock, &res))
        return FSE_CON_ERROR;

    return process_response(&res);
}

int fs_read(int server_handle, int fd, void *buf, size_t len)
{
    struct fs_response res;
    int sock, rcvd, response_len;

    if (INV_HANDLE(server_handle))
        return FSE_INVALID_HANDLE;
    sock = servers[server_handle].sock;

    if (-1 == write_command(sock, FSMSG_READ, fd, len, 0))
        return FSE_CON_ERROR;

    if (-1 == read_response(sock, &res))
        return FSE_CON_ERROR;
    if (res.status) {
        errno = res.status;
        return FSE_FAIL;
    }
    response_len = res.val;
    if (response_len > len)
        return FSE_INVALID_RESP;

    for (; response_len > 0; response_len -= rcvd) {
        rcvd = recv(sock, buf, response_len, 0);
        if (rcvd <= 0)
            return FSE_CON_ERROR;

        buf += rcvd;
    }

    return res.val;
}

int fs_lseek(int server_handle, int fd, long offset, int whence)
{
    struct fs_response res;
    int sock;

    if (INV_HANDLE(server_handle))
        return FSE_INVALID_HANDLE;
    sock = servers[server_handle].sock;

    write_command(sock, FSMSG_LSEEK, fd, offset, whence);
    read_response(sock, &res);
    return process_response(&res);
}

int fs_close(int server_handle, int fd)
{
    struct fs_response res;
    int sock;

    if (INV_HANDLE(server_handle))
        return FSE_INVALID_HANDLE;
    sock = servers[server_handle].sock;

    write_command(sock, FSMSG_CLOSE, fd, 0, 0);
    read_response(sock, &res);
    return process_response(&res);
}

int fs_fstat(int server_handle, int fd, struct stat *buf)
{
    return 0;
}

