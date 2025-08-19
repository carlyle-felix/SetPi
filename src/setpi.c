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
        printf("\n");
        printf("Usage:\n");
        printf("\tsetpi [action] <arg> ...\n");
        printf("\tsetpi [command] [action] <arg> ...\n");
        printf("\tsetpi [command] <arg>\n");
        printf("\n");
        printf("action:\n");
        printf("\t--set | -s\tadd or update current config item(s).\n");
        printf("\t--get | -g\tprint value of current config item(s).\n");
        printf("\n");
        printf("action examples:\tsetpi --set kernel=kernel8.img disable_overscan=1 ...\n");
        printf("\t\t\tsetpi -g arm_boost over_voltage_delta ...\n");
        printf("\n");
        printf("command:\n");
        printf("\tprofile\t\tmanage and set config profiles.\n");
        printf("\n");
        printf("profile example:\tsetpi profile overclock-profile\n");
        printf("\n");
        printf("profile action:\n");
        printf("\t--save | -S\tsave current config.txt to a specified profile name.\n");
        printf("\t--del  | -d\tdelete a specified profile.\n");
        printf("\t--new  | -n\tcreate a new profile using the current config as a base\n");
        printf("\t\t\tmodifying only specified keys.\n");
        printf("\n");
        printf("new profile example:\tsetpi profile --new underclock arm_freq=2000 gpu_freq=600\n");

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

        } else if (!strcmp(argv[2], "--del") || !strcmp(argv[2], "-d")) {
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