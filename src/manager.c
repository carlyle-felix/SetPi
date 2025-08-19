#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#include "../incl/manager.h"

#define SETPI "/etc/setpi/"
#define MAX_BUFFER 256
#define MAX_STR 128

struct node {
    char *key;
    char *value;
    struct node *next;
};

typedef struct fstab_boot {
    char *dev;
    char *mountpoint;
    char *fs;
} *Boot;

char *get_buffer(const char *p);
char *resize_buffer(char *buffer, uint16_t pos, uint8_t new_len, uint8_t cur_len);
void delete_boot_struct(Boot b);
void *mem_alloc(uint16_t n);
void *mem_realloc(void *p, uint16_t n);
int8_t write_config(const char *buffer, const char *path);
Boot read_fstab(void);
int8_t safe_mount(Boot boot);
int8_t is_mounted(char *mountpoint);
char *update_config(List l, char *buffer_);


List create_list(void)
{
    List l = mem_alloc(sizeof(struct node));
    if (!l) {
        return NULL;
    }
    l->key = NULL;
    l->value = NULL;
    l->next = NULL;

    return l;
}

void delete_list(List l)
{
    List temp;

    while (l) {
        temp = l;
        l = l->next;

        free(temp->key);
        free(temp->value);
        free(temp);
    }
}

void delete_boot_struct(Boot b)
{
    free(b->dev);
    free(b->mountpoint);
    free(b->fs);
    free(b);
}

void *mem_alloc(uint16_t n)
{
    void *p = malloc(n);
    if (!p) {
        printf("error: unable to allocate memory for new pointer.\n");
        return NULL;
    }

    return p;
}

void *mem_realloc(void *p, uint16_t n)
{
    void *new;

    new = realloc(p, n);
    if (!new) {
        free(p);
        printf("error: unable to reallocate memory for pointer.\n");
        return NULL;
    }

    return new;
}

List add_item(List l, const char *item)
{
    List temp;
    char str[MAX_STR];
    register uint8_t i;

    if (!l->key) {
        free(l);
        l = NULL;
    }
    
    temp = mem_alloc(sizeof(struct node));
    if (!temp) {
        return NULL;
    }

    for (i = 0; *item && *item != '='; i++) {
        str[i] = *item++;
    }
    str[i] = '\0';
    
    temp->key = mem_alloc(i + 1);
    if (!temp->key) {
        return NULL;
    }
    strcpy(temp->key, str);

    if (*item) {
        for (i = 0; *item++; i++) {
            str[i] = *item;
        }
        str[i] = '\0';

        temp->value = mem_alloc(i + 1);
        if (!temp->value) {
            return NULL;
        }
        strcpy(temp->value, str);
    } else {
        temp->value = NULL;
    }

    temp->next = l;
    l = temp;
    
    return l;
}

/*
    returns a pointer to the absolute path to config.txt
*/
char *config_path(void)
{
    FILE *fp;
    char *config;
    Boot boot;

    // check if mountpoint is /boot or /boot/firmware in fstab
    boot = read_fstab();

    if (safe_mount(boot)) {
        printf("error: unable to mount %s\n", boot->mountpoint);
        delete_boot_struct(boot);
        return NULL;
    }

    config = mem_alloc(strlen(boot->mountpoint) + strlen("/config.txt") + 1);
    if (!config) {
        delete_boot_struct(boot);
        return NULL;
    }
    sprintf(config, "%s/config.txt", boot->mountpoint);
    delete_boot_struct(boot);

    fp = fopen(config, "r");
    if (!fp) {
        printf("error: unable to open %s", config);
        free(config);
        return NULL;
    }
    fclose(fp);

    return config;
}

/*
    save current config.txt as a profile in /etc/setpi/
*/
int8_t save_profile(char *str)
{
    char *profile, *buffer, *config;
    int8_t status;

    config = config_path();
    if (!config) {
        return -1;
    }
    
    buffer = get_buffer(config);
    free(config);
    if (!buffer) {
        return -1;
    }

    profile = mem_alloc(strlen(SETPI) + strlen(str) + 1);
    if (!profile) {
        free(buffer);
        return -1;
    }

    status = write_config(buffer, profile);
    free(buffer);
    free(profile);

    return status;
}

/*
    create a new profile using the current config.txt
    as a base and applying specified values to it before 
    saving in /etc/setpi
*/
int8_t new_profile(List l, char *str)
{
    char *profile, *buffer, *config;
    int8_t status;

    config = config_path();
    if (!config) {
        return -1;
    }

    buffer = get_buffer(config);
    free(config);
    if (!buffer) {
        return -1;
    }

    buffer = update_config(l, buffer);
    if (!buffer) {
        return -1;
    }

    profile = mem_alloc(strlen(SETPI) + strlen(str) + 1);
    if (!profile) {
        return -1;
    }
    sprintf(profile, "%s%s", SETPI, str);

    status = write_config(buffer, profile);
    free(buffer);
    free(profile);

    return status;
}

/*
    delete a saved profile in /etc/setpi
*/
int8_t delete_profile(char *str)
{
    char *profile;
    int8_t status;

    profile = mem_alloc(strlen(SETPI) + strlen(str) + 1);
    if (!profile) {
        return -1;
    }
    sprintf(profile, "%s%s", SETPI, str);

    status = remove(profile);
    free(profile);

    return status;
}

/*
    overwrite /boot/config.txt or /boot/firmware/config/txt with
    a profile from /etc/setpi
*/
int8_t apply_profile(char *str)
{
    char *profile, *buffer, *config;
    int8_t status;

    profile = mem_alloc(strlen(SETPI) + strlen(str) + 1);
    if (!profile) {
        return -1;
    }
    sprintf(profile, "%s%s", SETPI, str);

    buffer = get_buffer(profile);
    free(profile);
    if (!buffer) {
        return -1;
    }

    config = config_path();
    if (!config) {
        free(buffer);
        return -1;
    }

    status = write_config(buffer, config);
    free(buffer);
    free(config);

    return status;
}

char *resize_buffer(char *buffer, uint16_t pos, uint8_t new_len, uint8_t cur_len)
{
    uint32_t buffer_len;
    register uint32_t i;
    uint8_t diff;
    char *str;

    buffer_len = strlen(buffer);
    if (new_len > cur_len) {
        diff = new_len - cur_len;

        // expand buffer.
        buffer = mem_realloc(buffer, buffer_len + diff + 1);
        if (!(buffer)) {
            return NULL;
        }
        str = buffer;

        buffer_len = buffer_len + diff;       // new buffer length.
        for (i = buffer_len; i > pos; i--) {
            str[i] = str[i - diff];
        }
    } else {
        str = buffer;
        diff = cur_len - new_len;

        for (i = pos; i < (buffer_len - diff); i++) {
            str[i] = str[i + diff];
        }
        str[i] = '\0';

        // shrink buffer.
        buffer = mem_realloc(buffer, buffer_len - diff + 1);
        if (!(buffer)) {
            return NULL;
        }
    }
    
    return buffer;
}

int8_t set_values(List l)
{
    char *config, *buffer;
    int8_t status;

    config = config_path();
    if (!config) {
        return -1;
    }

    buffer = get_buffer(config);
    if (!buffer) {
        free(config);
        return -1;
    }

    buffer = update_config(l, buffer);
    if (!buffer) {
        free(config);
        return -1;
    }

    status = write_config(buffer, config);
    free(config);
    free(buffer);

    return status;
}

/*
    returns a buffer with the updated config.
*/
char *update_config(List l, char *buffer_)
{
    List temp;
    register uint8_t i;
    uint8_t key_len;
    uint16_t len, pos;
    char *buffer, str[MAX_BUFFER];

    buffer = buffer_;
    for (temp = l; temp; temp = temp->next) {
        key_len = strlen(temp->key);
        buffer = buffer_;
        while (*buffer) {
            // ignore comments.
            if (*buffer == '#') {
                while (*buffer && *buffer++ != '\n');
                continue;
            }

            for (i = 0; *buffer == temp->key[i];) {
                buffer++;
                i++;
            }

            if (i == key_len && *buffer == '=') {
                buffer++;   // skip '='.
                for (i = 0; *buffer && *buffer++ != '\n'; i++);     // count characters betweeen '=' and '\n' (the value).

                while (*(buffer - 1) != '=') {
                    buffer--;
                }

                // resize the buffer and update pointers.
		        len = strlen(temp->value);
                if (len != i) {
                    pos = buffer - buffer_;
                    buffer_ = resize_buffer(buffer_, pos, len, i);
                    if (!buffer_) {
                        return NULL;
                    }
                    buffer = (buffer_ + pos);
		        } 

                for (i = 0; temp->value[i]; i++) {
                    *buffer++ = temp->value[i];
                }

		        break;

            } else {
                while (*buffer && *buffer++ != '\n');
                continue;
            }
        }
        
        // if key doesn't exist, prompt user to add it to the bottom of the file.
        if (!(*buffer)) {
            char c = 'a';

            printf("key %s not found in config, add %s=%s to config? [Y/n]: ", temp->key, temp->key, temp->value);
            while ((c = getchar())) {
                if (c == 'n' || c == 'N') {
                    break;
                } else if (c != 'y' && c != 'Y' && c != '\n') {
                    continue;
                }
                sprintf(str, "%s=%s\n", temp->key, temp->value);
                strcat(buffer_, str);
                break;
            }
        }
    }

    return buffer_;
}

int8_t write_config(const char *buffer, const char *path)
{
    FILE *fp;
    int16_t len;

    fp = fopen(path, "w");
    if (!fp) {
        printf("error: failed to open config.txt in write_config().\n");
        return -1;
    }

    // write buffer to config.txt
    len = fwrite(buffer, sizeof(char), strlen(buffer), fp);
    if (len < (int16_t) strlen(buffer)) {
        printf("error: failed to write new config.txt.\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    return 0;
}

List get_values(List l)
{
    List temp;
    char *config, *buffer, *buffer_, value[MAX_STR];
    register uint8_t i;

    config = config_path();
    if (!config) {
        return NULL;
    }

    buffer = get_buffer(config);
    free(config);
    if (!buffer) {
        return NULL;
    }

    buffer_ = buffer;
    for (temp = l; temp; temp = temp->next) {
        buffer = buffer_;
        while (*buffer) {
            if (*buffer == '#') {
                while (*buffer && *buffer++ != '\n');
            }

            for (i = 0; *buffer == temp->key[i]; i++) {
                buffer++;
            }

            if (i == strlen(temp->key) && *buffer == '=') {
                buffer++;   // skip '='

                for (i = 0; *buffer != '\n'; i++) {
                    value[i] = *buffer++;
                }
                value[i] = '\0';

                // copy config value into value member in list.
                temp->value = mem_alloc(i + 1);
                if (!temp->value) { 
                    return NULL;
                }
                strcpy(temp->value, value);

                break;
            } else {
                while (*buffer && *buffer++ != '\n');
                continue;
            }
        }
    }
    free(buffer_);

    return l;
}

void print_list(List l)
{
    List temp;

    for (temp = l; temp; temp = temp->next) {
        printf("%s", temp->key);
        if (temp->value) {
            printf("=%s\n", temp->value);
        } else {
            printf(" is not set in config.\n");
        }
    }
}

/*
    returns a malloc'd pointer to buffer of text found in config.txt.
*/
char *get_buffer(const char *p)
{
    FILE *fp;
    char *buffer;
    int read, n = MAX_BUFFER;

    buffer = mem_alloc(n);
    if (!buffer) {
        return NULL;
    }

    for (;;) {

        fp = fopen(p, "r");
        if (!fp) {
            printf("error: unable to open %s.\n", p);
            free(buffer);
            return NULL;
        }

        read = fread(buffer, sizeof(char), n, fp);
        if (read == n) {
            n *= 2;
            buffer = mem_realloc(buffer, n);
            if (!buffer) {
                fclose(fp);
                return NULL;
            }
            fclose(fp);
            continue;
        }

        fclose(fp);
        break;
    }
    buffer[read] = '\0';

    return buffer;
}

/*
    copy entries for dev, fs and mountpoint into fstab_boot struct.
    NOTE: caller must free fstab_boot struct.
*/
Boot read_fstab(void)
{
    char *buffer, *buffer_, str[MAX_STR] = "/boot";
    register uint8_t i;
    Boot boot;

    buffer = get_buffer("/etc/fstab");
    if (!buffer) {
        return NULL;
    }

    boot = mem_alloc(sizeof(struct fstab_boot));
    if (!boot) {
        free(buffer);
        return NULL;
    }

    buffer_ = buffer;
    while (*buffer) {
        if (*buffer == '#') {
            while (*buffer && *buffer++ != '\n');
        }

        for (i = 0; *buffer == str[i]; i++) {
            buffer++;
        }

        if (i == strlen(str)) {
            // check if mountpoint is /boot or /boot/firmware
            for (i = strlen(str); *buffer != '\t' && *buffer != ' '; i++) {
                str[i] = *buffer++;
            }
            str[i] = '\0';
            boot->mountpoint = mem_alloc(strlen(str) + 1);
            if (!boot->mountpoint) {
                free(buffer_);
                delete_boot_struct(boot);
                return NULL;
            }
            strcpy(boot->mountpoint, str);
            for (i = 0; str[i]; i++) {
                str[i] = '\0';
            }

            // find filesystem.
            while (*buffer == '\t' || *buffer == ' ') {
                buffer++;
            }
            
            for (i = 0; *buffer != '\t' && *buffer != ' '; i++) {
                str[i] = *buffer++;
            }
            str[i] = '\0';
            boot->fs = mem_alloc(strlen(str) + 1);
            if (!boot->fs) {
                free(buffer_);
                delete_boot_struct(boot);
                return NULL;
            }
            strcpy(boot->fs, str);
            for (i = 0; str[i]; i++) {
                str[i] = '\0';
            }
        
            // find partition to mount.
            while (*(buffer - 1) != '\n') {
                buffer--;
            }

            for (i = 0; *buffer != '\t' && *buffer != ' '; i++) {
                str[i] = *buffer++;
            }
            str[i] = '\0';
            boot->dev = mem_alloc(strlen(str) + 1);
            if (!boot->dev) {
                free(buffer_);
                delete_boot_struct(boot);
                return NULL;
            }
            strcpy(boot->dev, str);
            for (i = 0; str[i]; i++) {
                str[i] = '\0';
            }

            break;

        } else {
            while (i--) {
                buffer--;
            }
        }
        buffer++;
    }
    free(buffer_);

    return boot;
}

int8_t safe_mount(Boot boot) 
{
    int8_t status;

    status = is_mounted(boot->mountpoint);

    if (status > 0) {
        return 0;
    } else if (status < 0) {
        return -1;
    }

    return mount(boot->dev, boot->mountpoint, boot->fs, 0, NULL);
}

int8_t is_mounted(char *mountpoint)
{
    char *buffer, *buffer_;
    register uint8_t i;

    buffer = get_buffer("/proc/mounts");
    if (!buffer) {
        return -1;
    }

    buffer_ = buffer;
    while (*buffer++) {
        for (i = 0; *buffer == mountpoint[i]; i++) {
            buffer++;
        }

        if (i == strlen(mountpoint)) {
            free(buffer_);
            return 1;
        }
    }

    free(buffer_);
    return 0;
}