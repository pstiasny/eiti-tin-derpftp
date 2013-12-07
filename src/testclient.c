#include <stdio.h>

#include "api.h"


int main(int argc, char **argv)
{
    printf("fs_open_server returned %d\n", fs_open_server(""));

    return 0;
}
