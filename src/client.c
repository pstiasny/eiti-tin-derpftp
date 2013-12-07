#include "api.h"
#include "types.h"

#define MAX_SERVERS 32
static struct serverd servers[MAX_SERVERS];

int fs_open_server(char *server_addr)
{
    return 0;
}

int fs_close_server(int server_handle)
{
    return 0;
}

int fs_open(int server_handle, char *name, int flags)
{
    return 0;
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

