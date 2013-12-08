#include <stdio.h>

#include "api.h"


int main(int argc, char **argv)
{
    int server_handle;
    char input_buf[1024], filename[256];

    server_handle = fs_open_server((argc < 2) ? "localhost" : argv[1]);
    printf("fs_open_server returned %d\n", server_handle);

    while (fgets(input_buf, sizeof(input_buf), stdin)) {
        if (*input_buf == '\n')
            continue;

        if (1 == sscanf(input_buf, "open %255s", filename)) {
            printf("fs_open returned %d\n",
                   fs_open(server_handle, filename, 16));
        } else {
            printf("unknown command\n");
        }
    }


    printf("fs_close_server returned %d\n", fs_close_server(server_handle));

    return 0;
}
