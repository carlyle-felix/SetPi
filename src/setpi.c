#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../incl/manager.h"

int main(int argc, char *argv[])
{
    List l;
    char *str;
    register uint8_t i;
    int8_t status;

    if (argc == 1 || !strcmp(argv[1], "--help")) {

        printf("SetPi:\tA CLI program for managing Raspberry Pi's config.txt\n");
        printf("\n");
        printf("usage:\n");
        printf("\tsetpi command [action] <arg> ...\n");
        printf("\n\trun \e[1msetpi profile\e[m to get the currently applied profile, if any.\n");
        printf("\n");
        printf("commands:\n");
        printf("\tconfig\t\tview and set config key(s).\n");
        printf("\tprofile\t\tmanage and set config profiles.\n");
        printf("\n");
        printf("[actions] for config:\n");
        printf("\tsetpi config [action] <key [value]> ...\n\n");
        printf("\t--set | -S\tadd or update current config item(s).\n");
        printf("\t--get | -g\tprint value of current config item(s).\n");
        printf("\n");
        printf("examples:\tsetpi config --set kernel kernel8.img disable_overscan 1 ...\n");
        printf("\t\tsetpi config -g arm_boost over_voltage_delta ...\n");
        printf("\n\n");
        printf("[actions] for profile:\n");
        printf("\tsetpi profile [action] <filename>\n\n");
        printf("\t--set  | -S\tset user created profile (see --new, --save).\n");
        printf("\t--list | -l\tprint a list of available profiles.\n");
        printf("\t--save | -s\tsave current config.txt to a specified profile name.\n");
        printf("\t--del  | -d\tdelete a specified profile.\n");
        printf("\t--new  | -n\tcreate a new profile using the current config as a base\n");
        printf("\t\t\tmodifying only specified keys.\n");
        printf("\n");
        printf("examples:\tsetpi profile --new underclock arm_freq 2000 gpu_freq 600\n");
        printf("\t\tsetpi profile --save my-profile\n");
        printf("\t\tsetpi profile --set my-profile\n");
        printf("\n");
        printf("bash-completion only suggests a list of common keys, for a full list\n");
        printf("see https://www.raspberrypi.com/documentation/computers/config_txt.html\n");
        printf("\n");

        return 0;
    } else {
        // check privilege
        if (getuid())
        {
            printf("error: pi-freq needs root privileges.\n");
            return -1;
        }
    }

    if (!strcmp(argv[1], "config")) {      
        if (argc == 2) {
            printf("error: no action specified, use --help for help.\n");
        } else if (!strcmp(argv[2], "--set") || !strcmp(argv[2], "-S")) {          
            l = create_list();
            for (i = 3; i < argc; i++) {
                l = add_item(l, argv[i], argv[i + 1]);
                if (!l) {
                    printf("error: unable to add key %s and value %s to list.\n", argv[i - 1], argv[i]);
                    delete_list(l);
                    return -1;
                }
                i++;
            }

            status = set_values(l);
            delete_list(l);
            if (status) {
                return status;
            }

        } else if (!strcmp(argv[2], "--get") || !strcmp(argv[2], "-g")) {  
            l = create_list();
            for (i = 3; i < argc; i++) {
                l = add_item(l, argv[i], NULL);
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
        } else {
            printf("unkown action: %s, use --help for help.\n", argv[2]);
        }
    } else if (!strcmp(argv[1], "profile")) {
        if (argc == 2) {
            str = current_profile();
            printf("\ncurrent profile: ");
            if (!str) {
                printf("no profile set.\n\n");
                free(str);
                return -1;
            }
            printf("%s\n\n", str);
            free(str);

        } else if (!strcmp(argv[2], "--set") || !strcmp(argv[2], "-S")) {
            status = apply_profile(argv[3]);
            if (status) {
                printf("error: unable to set profile %s.\n", argv[3]);
            }

            return status;

        } else if (!strcmp(argv[2], "--save") || !strcmp(argv[2], "-s")) {
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
                l = add_item(l, argv[i], NULL);
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

        } else if (!strcmp(argv[2], "--list") || !strcmp(argv[2], "-l")) {
            status = profile_list();
            if (status) {
                printf("error: unable to retrieve list of profiles.\n");
            }

            return status;
        } else {
            printf("unkown action: %s, use --help for help.\n", argv[2]);
        }

    } else {
        printf("unkown command: %s, use --help for help.\n", argv[1]);
        return -1;
    }

    return 0;
}