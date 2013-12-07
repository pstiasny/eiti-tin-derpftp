#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int fs_open_server(char *server_addr);
int fs_close_server(int server_handle);
int fs_open(int server_handle, char *name, int flags);
int fs_write(int server_handle, int fd, void *buf, size_t len);
int fs_read(int server_handle, int fd, void *buf, size_t len);
int fs_lseek(int server_handle, int fd, long offset, int whence);
int fs_close(int server_handle, int fd);
int fs_fstat(int server_handle, int fd, struct stat *buf);

