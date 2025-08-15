#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../incl/manager.h"

Key enum_key(char *arg);

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
        k = enum_key(argv[2]);
        if (k == INVALID) {
            printf("error: invalid key %s.\n", argv[2]);
            return -1;
        }
        printf("%s", current_value(k));

        return 0;
    }

    l = create_list();
    for (i = 1; i < argc; i++) {
	k = enum_key(argv[i]);
        if (k == INVALID) {
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

Key enum_key(char *arg)
{
    if (!strcmp(arg, "cpu")) {
        return ARM;
    } else if (!strcmp(arg, "gpu")) {
        return GPU;
    } else if (!strcmp(arg, "ov")) {
        return OV;
    }

    return INVALID;
}
