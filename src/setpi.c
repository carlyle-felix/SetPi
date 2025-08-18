#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    register int8_t i;

    // check privilege
    if (getuid())
    {
        printf("error: pi-freq needs root privileges.\n");
        return -1;
    }

    if (argc == 1) {
        printf("menu goes here.\n");
        return 0;
    } else {
        l = create_list();
        for (i = 2; i < argc; i++) {
            l = add_item(l, argv[i]);
            if (!l) {
                printf("failed at %s.\n", argv[i]);
                delete_list(l);
                return -1;
            }
        }
    }
    
    if (!strcmp(argv[1], "--set")) {
        i = write_config(l);
        if (i) {
            delete_list(l);
            return -1;
        }
    } else if (!strcmp(argv[1], "--get")) {
        l = get_values(l);
        if (!l) {
            return -1;
        }
        print_list(l);
    }

    delete_list(l);

    return 0;
}