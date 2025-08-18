#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#include "../incl/manager.h"

#define MAX_BUFFER 256
#define MAX_STR 128

struct node {
    char *key;
    char *value;
    struct node *next;
};

char *get_buffer(const char *p);
void resize_buffer(char *buffer, uint16_t pos, uint8_t new_len, uint8_t cur_len);
void *mem_alloc(uint16_t n);
int8_t mount_part(const char *mountpoint);
int8_t is_mounted(char *mountpoint);

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
    returns a pointer with the absolute path to config.txt
*/
char *config_path(void)
{
    FILE *fp;
    char *p;

    p = mem_alloc(MAX_STR);
    if (!p) {
        return NULL;
    }
    
    // /boot/firmware/config.txt", "r"
    if ((fp = fopen("/boot/firmware/config.txt", "r"))) {
        strcpy(p, "/boot/firmware/config.txt");
    } else if ((fp = fopen("/boot/config.txt", "r"))) {
        strcpy(p, "/boot/config.txt");
    } else {
        printf("error: unable to locate config.txt.\n");
        free(p);
        return NULL;
    }
    fclose(fp);

    return p;
}

int8_t write_config(List l)
{
    FILE *fp;
    List temp;
    register uint8_t i;
    uint8_t key_len;
    uint16_t len;
    char *buffer, *path, *buffer_, str[MAX_BUFFER];
    
    if (mount_part("/boot")) {
        printf("error: unable to mount /boot\n");
        return -1;
    }
    
    path = config_path();
    if (!path) {
        return -1;
    }

    buffer = get_buffer(path);
    if (!buffer) {
        free(path);
        return -1;
    }

    // allocate additional MAX_BUFFER bytes for changes.
    buffer = mem_realloc(buffer, strlen(buffer) + MAX_BUFFER);
    if (!buffer) {
        free(path);
        return -1;
    }

    buffer_ = buffer;
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

                // resize the buffer.
		        len = strlen(temp->value);
                if (len != i) {
                    resize_buffer(buffer_, (buffer - buffer_), len, i);
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

    fp = fopen(path, "w");
    free(path);
    if (!fp) {
        printf("error: failed to open config.txt in write_config().\n");
        free(buffer_);
        return -1;
    }

    // write buffer to config.txt
    len = fwrite(buffer_, sizeof(char), strlen(buffer_), fp);
    if (len < strlen(buffer_)) {
        free(buffer_);
        printf("error: failed to write new config.txt.\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    free(buffer_);

    if (umount("/boot")) {
        printf("info: failed to unmount /boot.\n");
    }

    return 0;
}

/*
    move buffer left or right after a specified position, expanding or truncating 
    the buffer to accomodate the new value.
*/
void resize_buffer(char *buffer, uint16_t pos, uint8_t new_len, uint8_t cur_len)
{
    uint32_t buffer_len;
    register uint32_t i;
    uint8_t diff;
    char *str = buffer;

    buffer_len = strlen(buffer);

    if (new_len > cur_len) {
        diff = new_len - cur_len;
        for (i = buffer_len + diff; i > pos; i--) {
            str[i] = str[i - diff];
        }
        str[buffer_len + diff + 1] = '\0';
    } else {
        diff = cur_len - new_len;
        for (i = pos; i < (buffer_len - diff); i++) {
            str[i] = str[i + diff];
        }
        str[i] = '\0';
    }
    
}

List get_values(List l)
{
    List temp;
    char *path, *buffer, *buffer_, value[MAX_STR];
    register uint8_t i;

    if (mount_part("/boot")) {
        printf("error: unable to mount /boot\n");
        return NULL;
    }

    path = config_path();
    if (!path) {
        return NULL;
    }

    buffer = get_buffer(path);
    free(path);
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

    if (umount("/boot")) {
        printf("info: unable to unmount /boot.\n");
    }

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

int8_t mount_part(const char *mountpoint) 
{
    char *buffer, *buffer_, fs[MAX_STR], dev[MAX_STR];
    register uint8_t i;
    int8_t status;

    status = is_mounted("/boot");

    if (status > 0) {
        return 0;
    } else if (status < 0) {
        return -1;
    }

    buffer = get_buffer("/etc/fstab");
    if (!buffer) {
        return -1;
    }

    buffer_ = buffer;
    while (*buffer) {
        if (*buffer == '#') {
            while (*buffer && *buffer++ != '\n');
        }

        for (i = 0; *buffer == mountpoint[i]; i++) {
            buffer++;
        }

        if (i == strlen(mountpoint)) {
            // find filesystem.
            while (*buffer == '\t' || *buffer == ' ') {
                buffer++;
            }
            
            for (i = 0; *buffer != '\t' && *buffer != ' '; i++) {
                fs[i] = *buffer++;
            }
            fs[i] = '\0';

            // find partition to mount.
            while (*(buffer - 1) != '\n') {
                buffer--;
            }

            for (i = 0; *buffer != '\t' && *buffer != ' '; i++) {
                dev[i] = *buffer++;
            }
            dev[i] = '\0';

            break;

        } else {
            while (i--) {
                buffer--;
            }
        }
        buffer++;
    }
    free(buffer_);

    return mount(dev, mountpoint, fs, 0, NULL);
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