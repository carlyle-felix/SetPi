#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    register int8_t i;

    if (argc == 1 || !strcmp(argv[1], "--help")) {

        printf("SetPi:\tA CLI program for managing Raspberry Pi's config.txt\n");
        printf("Usage:\tsetpi [option] [<args>] ...\n");
        printf("options:\n");
        printf("\t--set | -s:\tadd or update config item(s).\n");
        printf("\t--get | -g:\tprint current value of config item(s).\n");
        printf("\n");
        printf("example:\n");
        printf("\tsetpi -s arm_freq=3000 gpu_freq=1000\n");

        return 0;
    } else {
        // check privilege
        if (getuid())
        {
            printf("error: pi-freq needs root privileges.\n");
            return -1;
        }

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
    
    if (!strcmp(argv[1], "--set") || !strcmp(argv[1], "-s")) {
        i = write_config(l);
        if (i) {
            delete_list(l);
            return -1;
        }
    } else if (!strcmp(argv[1], "--get") || !strcmp(argv[1], "-g")) {
        l = get_values(l);
        if (!l) {
            return -1;
        }
        print_list(l);
    }

    delete_list(l);

    return 0;
}