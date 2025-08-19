#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    register uint8_t i;
    int8_t status;

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
    }
    
    if (!strcmp(argv[1], "--set") || !strcmp(argv[1], "-s")) {
        
        l = create_list();
        for (i = 2; i < argc; i++) {
            l = add_item(l, argv[i]);
            if (!l) {
                printf("error: unable to add item %s to list.\n", argv[i]);
                delete_list(l);
                return -1;
            }
        }

        status = set_values(l);
        delete_list(l);
        if (status) {
            return status;
        }

    } else if (!strcmp(argv[1], "--get") || !strcmp(argv[1], "-g")) {
        
        l = create_list();
        for (i = 2; i < argc; i++) {
            l = add_item(l, argv[i]);
            if (!l) {
                printf("error: unable to add item %s to list.\n", argv[i]);
                delete_list(l);
                return -1;
            }
        }

        l = get_values(l);
        if (!l) {
            printf("error: unable to get values from config.txt.\n");
            return -1;
        }
        print_list(l);
        delete_list(l);

    } else if (!strcmp(argv[1], "profile")) {
        
        if (!strcmp(argv[2], "--save") || !strcmp(argv[2], "-S")) {
            status = save_profile(argv[3]);
            if (status) {
                printf("error: unable to save profile %s.\n", argv[3]);
            }

            return status;

        } else if (!strcmp(argv[2], "--delete") || !strcmp(argv[2], "-d")) {
            status = delete_profile(argv[3]);
            if (status) {
                printf("error: unable to delete profile %s.\n", argv[3]);
            }

            return status;
            
        } else if (!strcmp(argv[2], "--new") || !strcmp(argv[2], "-n")) {
            l = create_list();
            for (i = 4; i < argc; i++) {
                l = add_item(l, argv[i]);
                if (!l) {
                    printf("error: unable to add item %s to list.\n", argv[i]);
                    delete_list(l);
                    return -1;
                }
            }

            status = new_profile(l, argv[3]);
            delete_list(l);
            if (status) {
                printf("error: unable to create new profile %s.\n", argv[3]);
            }

            return status;
        } 

        status = apply_profile(argv[2]);
        if (status) {
            printf("error: unable to apply profile %s.\n", argv[3]);
        }
    }

    return 0;
}