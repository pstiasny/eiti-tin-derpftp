#include "api.h"
#include "types.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX_SERVERS 32
#define INV_HANDLE(h) ((h < 0) || (h > MAX_SERVERS) || !servers[h].in_use)
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

int fs_open_server(char *server_addr)
{
    int server_handle;
    struct serverd *sd;
    struct sockaddr_in addr;
    struct hostent *hp;

    if (-1 == (server_handle = get_free_handle()))
        return FSE_CON_LIMIT;
    sd = &servers[server_handle]; 

    /* resolve the name */
    hp = gethostbyname(server_addr);
    if (hp == 0)
        return FSE_UNKNOWN_HOST;
    memcpy((char*)&addr.sin_addr, (char*)hp->h_addr, hp->h_length);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1337);

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
    /* TODO: connection finalization? */
}

int fs_open(int server_handle, char *name, int flags)
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

    if (res.status) {
        errno = res.status;
        return FSE_FAIL;
    } else {
        return res.val;
    }
}

int fs_write(int server_handle, int fd, void *buf, size_t len)
{
    return 0;
}

int fs_read(int server_handle, int fd, void *buf, size_t len)
{
    return 0;
}

int fs_lseek(int server_handle, int fd, long offset, int whence)
{
    return 0;
}

int fs_close(int server_handle, int fd)
{
    return 0;
}

int fs_fstat(int server_handle, int fd, struct stat *buf)
{
    return 0;
}

