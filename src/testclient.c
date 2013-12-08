#include <stdio.h>

#include "api.h"


int main(int argc, char **argv)
{
    int server_handle = fs_open_server("localhost");
    printf("fs_open_server returned %d\n", server_handle);
    printf("fs_open returned %d\n",
        fs_open(server_handle, "Hello world from da client lib!\n", 16));
    printf("fs_close_server returned %d\n", fs_close_server(server_handle));

    return 0;
}
