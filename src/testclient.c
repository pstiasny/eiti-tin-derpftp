#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/fs_server.h"


int main(int argc, char **argv)
{
    int server_handle, len, fd, remote_fd, result, offset, whence;
    char input_buf[1024], filename[256];

    server_handle = fs_open_server((argc < 2) ? "localhost" : argv[1]);
    printf("fs_open_server returned %d\n", server_handle);

    while (fgets(input_buf, sizeof(input_buf), stdin)) {
        if (*input_buf == '\n')
            continue;

        if (1 == sscanf(input_buf, "open %255s", filename)) {
            remote_fd = fs_open(server_handle, filename, O_RDWR | O_CREAT);
            printf("fs_open returned %d\n", remote_fd >= 0);
            if (remote_fd == FSE_FAIL)
                perror("fs_open");

        } else if (2 == sscanf(input_buf, "write %255s %d", filename, &len)) {
            fd = open(filename, O_RDONLY);

            char fbuf[len];
            int read_ = read(fd, fbuf, len);
            close(fd);

            result = fs_write(server_handle, remote_fd, fbuf, read_);
            printf("fs_write returned %d\n", result);
            if (result == FSE_FAIL)
                perror("fs_write");

        } else if (2 == sscanf(input_buf, "read %255s %d", filename, &len)) {
            char fbuf[len];
            result = fs_read(server_handle, remote_fd, fbuf, len);

            if (result >= 0) {
                fd = open(filename, O_WRONLY | O_CREAT, 0660);
                write(fd, fbuf, result);
                close(fd);
            }

            printf("fs_read returned %d\n", result);
            if (result == FSE_FAIL)
                perror("fs_read");

        } else if (2 == sscanf(input_buf, "lseek %d %d", &offset, &whence)) {
            result = fs_lseek(server_handle, remote_fd, offset, whence);

            printf("fs_lseek returned %d\n", result);
            if (result == FSE_FAIL)
                perror("fs_lseek");

        } else if (0 == strcmp(input_buf, "fstat\n")) {
            struct fs_stat stat;
            result = fs_fstat(server_handle, remote_fd, &stat);

            printf("fs_fstat returned %d\n", result);
            printf("mode: 0%o\n", stat.st_mode & 0x777);
            printf("size: %d\n", stat.st_size);
            if (result == FSE_FAIL)
                perror("fs_close");

        } else if (0 == strcmp(input_buf, "close\n")) {
            result = fs_close(server_handle, remote_fd);

            printf("fs_close returned %d\n", result);
            if (result == FSE_FAIL)
                perror("fs_close");

        } else {
            printf("unknown command\n");
        }
    }


    printf("fs_close_server returned %d\n", fs_close_server(server_handle));

    return 0;
}
