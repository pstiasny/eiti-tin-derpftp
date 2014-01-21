#ifndef FS_SERVER_H
#define FS_SERVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FSE_OK              (0)
#define FSE_FAIL            (-1)
#define FSE_CON_LIMIT       (-2)
#define FSE_CON_ERROR       (-3)
#define FSE_UNKNOWN_HOST    (-4)
#define FSE_INVALID_HANDLE  (-5)
#define FSE_INVALID_RESP    (-6)

struct fs_stat {
    int32_t st_size;
    int32_t st_atim;
    int32_t st_mtim;
    int32_t st_ctim;
    int32_t st_blocks;
    int32_t st_blksize;
    int16_t st_mode;
};

int fs_open_server(const char *server_addr);
int fs_close_server(int server_handle);
int fs_open(int server_handle, const char *name, int flags);
int fs_write(int server_handle, int fd, void *buf, size_t len);
int fs_read(int server_handle, int fd, void *buf, size_t len);
int fs_lseek(int server_handle, int fd, long offset, int whence);
int fs_close(int server_handle, int fd);
int fs_fstat(int server_handle, int fd, struct fs_stat *buf);

#endif
