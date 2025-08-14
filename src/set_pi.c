#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    uint8_t i;
    // check privilege
    if (!getuid())
    {
        printf("error: pi-freq needs root privileges.\n");
        return 1;
    }

    if (argc == 1) {
        printf("menu goes here.\n");
    } else if (!strcmp(argv[1], "arm_freq")) {
        l = create_list();
        i = set_value(l, ARM, argv[2]);
        if (i) {
            printf("failed.\n");
            return -1;
        }
        write_config(l);
        delete_list(l);
    } 

    return 0;
}