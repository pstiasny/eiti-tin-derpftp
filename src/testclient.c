#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "api.h"


int main(int argc, char **argv)
{
    int server_handle, len, remote_fd;
    char input_buf[1024], filename[256];

    server_handle = fs_open_server((argc < 2) ? "localhost" : argv[1]);
    printf("fs_open_server returned %d\n", server_handle);

    while (fgets(input_buf, sizeof(input_buf), stdin)) {
        if (*input_buf == '\n')
            continue;

        if (1 == sscanf(input_buf, "open %255s", filename)) {
            remote_fd = fs_open(server_handle, filename, O_RDWR | O_CREAT);
            printf("fs_open returned %d\n", remote_fd);
            if (remote_fd == FSE_FAIL)
                perror("fs_open");

        } else if (2 == sscanf(input_buf, "write %255s %d", filename, &len)) {
            printf("reading %s\n", filename);
            int fd = open(filename, O_RDONLY);

            char fbuf[len];
            int read_ = read(fd, fbuf, len);
            close(fd);

            int result = fs_write(server_handle, remote_fd, fbuf, read_);
            printf("fs_write returned %d\n", result);
            if (result == FSE_FAIL)
                perror("fs_write");

        } else {
            printf("unknown command\n");
        }
    }


    printf("fs_close_server returned %d\n", fs_close_server(server_handle));

    return 0;
}
