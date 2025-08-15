#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    register uint8_t i;
    uint8_t j;
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
    }

    if (!strcmp(argv[1], "show")) {
        if (!strcmp(argv[2], "cpu")) {
            k = ARM;
        } else if (!strcmp(argv[2], "gpu")) {
            k = GPU;
        } else if (!strcmp(argv[2], "ov")) {
            k = OV;
        }
        printf("%s", current_value(k));

        return 0;
    }

    l = create_list();
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "cpu")) {
            k = ARM;
        } else if (!strcmp(argv[i], "gpu")) {
            k = GPU;
        } else if (!strcmp(argv[i], "ov")) {
            k = OV;
        } else {
            continue;
        }
        j = set_value(l, k, argv[++i]);
        if (j) {
            printf("failed.\n");
            return -1;
        }
    }

    write_config(l);
    delete_list(l);

    return 0;
}
