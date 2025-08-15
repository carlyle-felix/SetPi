#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    uint8_t i;
    Key k;

    // check privilege
    if (!getuid())
    {
        printf("error: pi-freq needs root privileges.\n");
        return 1;
    }

    if (argc == 1) {
        printf("menu goes here.\n");
	return 0;
    } else if (!strcmp(argv[1], "cpu")) {
        k = ARM;
    } else if (!strcmp(argv[1], "gpu")) {
        k = GPU;
    } else if (!strcmp(argv[1], "ov")) {
        k = OV;
    }

    l = create_list();
    i = set_value(l, k, argv[2]);
    if (i) {
        printf("failed.\n");
        return -1;
    }
    write_config(l);
    delete_list(l);

    return 0;
}
