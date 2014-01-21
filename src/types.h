#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


enum FSMSG_TYPE {
    FSMSG_OPEN,
    FSMSG_WRITE,
    FSMSG_READ,
    FSMSG_LSEEK,
    FSMSG_CLOSE,
    FSMSG_STAT,
};

struct fs_command {
    int8_t type;
    int32_t fd, arg1, arg2;
};

struct fs_open_command {
    struct fs_command base_command;
    char filename[256];
};

struct fs_response {
    int32_t status;
    int32_t val;
};

struct fs_stat_response {
    struct fs_response base_response;
    struct {
        int16_t st_mode;
        int32_t st_size;
        int32_t st_atim;
        int32_t st_mtim;
        int32_t st_ctim;
        int32_t st_blocks;
        int32_t st_blksize;
    } stat;
};

