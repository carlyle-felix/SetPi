#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../incl/manager.h"

#define KEYS {"arm_freq", "gpu_freq", "over_voltage_delta"}
#define MAX_BUFFER 256
#define MAX_STR 128

struct value_list {
    char *arm_freq;
    char *gpu_freq;
    char *ov;
};

typedef struct {
    char *fs;
    char *part;
} Dev;

char *get_buffer(const char *p);
void truncate(char *buffer, uint16_t pos, uint8_t t_len);
void expand(char *buffer, uint16_t pos, uint8_t t_len);
void *mem_alloc(uint16_t n);
char *find_fs(const char *mp);

List create_list(void)
{
    List l = mem_alloc(sizeof(struct value_list));
    if (!l) {
        return NULL;
    }
    l->arm_freq = NULL;
    l->gpu_freq = NULL;
    l->ov = NULL;

    return l;
}

void delete_list(List l)
{
    free(l->arm_freq);
    free(l->gpu_freq);
    free(l->ov);
    free(l);
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

int8_t set_value(List l, Key k, const char *value)
{
    switch (k) {

        case ARM:
            l->arm_freq = mem_alloc(strlen(value) + 1);
            if (!l->arm_freq) {
                return -1;
            }
            strcpy(l->arm_freq, value);
            break;

        case GPU:
            l->gpu_freq = mem_alloc(strlen(value) + 1);
            if (!l->gpu_freq) {
                return -1;
            }
            strcpy(l->gpu_freq, value);
            break;

        case OV:
            l->ov = mem_alloc(strlen(value) + 1);
            if (!l->ov) {
                return -1;
            }
            strcpy(l->ov, value);
            break;

        default:
            break;
    }

    return 0;
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
    if ((fp = fopen("/home/carlyle/config.txt", "r"))) {
        strcpy(p, "/home/carlyle/config.txt");
    } else if ((fp = fopen("/boot/config.txt", "r"))) {
        strcpy(p, "/boot/config.txt");
    } else {
        printf("error: unable to locate config.txt.\n");
        return NULL;
    }
    fclose(fp);

    return p;
}

int8_t write_config(List l)
{
    FILE *fp;
    register uint8_t i, j, k;
    uint8_t num_keys, key_len, len;
    const char *key[] = KEYS;
    char *buffer, *path, *buffer_, value[MAX_STR], str[MAX_BUFFER];

    path = config_path();
    if (!path) {
        return -1;
    }

    buffer = get_buffer(path);
    free(path);
    if (!buffer) {
        return -1;
    }

    // allocate additional MAX_BUFFER bytes for changes.
    buffer = realloc(buffer, strlen(buffer) + MAX_BUFFER);
    if (!buffer) {
        printf("error: failed to reallocate memory buffer in write_config().\n");
        return -1;
    }

    num_keys = (uint8_t) (sizeof(key) / sizeof(key[0]));
    buffer_ = buffer;
    for (i = 0; i < num_keys; i++) {
        key_len = strlen(key[i]);

        // if a value in the list is empty, skip it.
        switch (i) {

            case 0:
                if (!l->arm_freq) {
                    continue;
                }

                strcpy(value, l->arm_freq);
                break;

            case 1:
                if (!l->gpu_freq) {
                    continue;
                }

                strcpy(value, l->gpu_freq);
                break;

            case 2:
                if (!l->ov) {
                    continue;
                }

                strcpy(value, l->ov);
                break;

            default:
                break;
        }

        buffer = buffer_;
        while (*buffer) {
            // ignore comments
            if (*buffer == '#') {
                while (*buffer && *buffer++ != '\n');
                continue;
            }

            j = 0;
            while (*buffer == key[i][j]) {
                buffer++;
                j++;
            }

            k = 0;
            if (j == key_len) {
                buffer++;   // go to character after '='
                while (*buffer && *buffer++ != '\n') {
                    k++;
                }

                while (*buffer != '=') {
                    buffer--;
                }
		buffer++;

                // truncate or expand the buffer
		j = 0;
		len = strlen(value);
                if (len > k) {
                    expand(buffer_, buffer - buffer_, (len - k));
		} else if (len < k) {
                    truncate(buffer_, (buffer + len) - buffer_, k - len);
     		}

                j = 0;
                while (value[j]) {
                    *buffer++ = value[j++];
                }

		break;

            } else {
                while (*buffer && *buffer++ != '\n');
                continue;
            }

	    if (!(*buffer)) {
		sprintf(str, "\n%s=%s", key[i], value);
		strcat(buffer_, str);
		break;
	    }
        }
    }

	printf("%s\n", buffer_);
 /*
    // TODO: gain root here.
    fp = fopen(path, "w");
    free(path);
    if (!fp) {
        printf("error: failed to open config.txt in write_config().\n");
        free(str);
        return -1;
    }

    len = fwrite(buffer, sizeof(char), strlen(buffer) + 1, fp);
    if (len < (strlen(buffer) + 1)) {
        printf("error: failed to write new config.txt.\n");
        free(str);
        return -1;
    }
    fclose(fp);
    // TODO: drop root here.
*/

    free(buffer_);  // buffer placeholder

    return 0;
}

void truncate(char *buffer, uint16_t pos, uint8_t t_len)
{
    uint32_t b_len;
    register uint32_t i;
    char *str = buffer;

    b_len = strlen(buffer);
    for (i = pos; i < (b_len - t_len); i++) {
        str[i] = str[i + t_len];
    }
    str[i] = '\0';
}

void expand(char *buffer, uint16_t pos, uint8_t e_len)
{
    uint32_t b_len;
    register uint32_t i;
    char *str = buffer;

    b_len = strlen(buffer);
    for (i = b_len + e_len; i > pos; i--) {
        str[i] = str[i - e_len];
    }
    str[b_len + e_len + 1] = '\0';
}

/*
        returns a malloc'd pointer to the value assigned to a given key.
*/
char *current_value(Key k)
{
    const char *key[] = KEYS;
    char *path, *buffer, *buffer_, value[MAX_STR], *str;
    register uint8_t i;

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
    while (*buffer) {
        if (*buffer == '#') {
            while (*buffer && *buffer++ != '\n');
        }

        i = 0;
        while (*buffer == key[k][i]) {
            buffer++;
            i++;
        }

        if (i == strlen(key[k])) {

            for (i = 0; *buffer++ != '\n'; i++) {
                value[i] = *buffer;
            }
            value[i] = '\0';

            break;
        } else {
            while (*buffer && *buffer++ != '\n');
            continue;
        }

    }
    if (!(*buffer)) {
        free(buffer_);
        return "key not found.";
    }

    str = mem_alloc(strlen(value) + 1);
    if (!str) {
        free(buffer_);
        return NULL;
    }
    strcpy(str, value);

    free(buffer_);

    return str;
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
        if (!p) {
            printf("error: unable to open %s.\n", p);
            return NULL;
        }

        read = fread(buffer, sizeof(char), n, fp);
        if (read == n) {
            n *= 2;
            buffer = realloc(buffer, n);
            if (!buffer) {
                printf("error: unable to reallocate memory for buffer in get_buffer().\n");
                return NULL;
            }
            continue;
        }

        fclose(fp);
        break;
    }
    buffer[read] = '\0';

    return buffer;
}

Dev *get_dev(const char *mp)
{
	char *buffer, *buffer_, str[MAX_STR];
        register uint8_t i;
        Dev *d;

        buffer = get_buffer("/etc/fstab")
        if (!buffer) {
            return NULL;
        }

        d = mem_alloc(sizeof(d));
        if (!d) {
            free(buffer);
            return NULL;
        }

        buffer_ = buffer;
        while (*buffer) {
            if (*buffer == '#') {
                while (*buffer && *buffer++ != '\n');
            }

            for (i = 0; *buffer == mp[i]; i++) {
                buffer++;
            }

            if (i == strlen(mp)) {
                while (*buffer++ == ' ');

                for (i = 0; *buffer != ' '; i++) {
                    str[i] = *buffer++;
                }
                str[i] = '\0';

                d.->fs = mem_alloc(strlen(str) + 1);
                if (!d->fs) {
                    free(buffer_);
                    free(d);
                    return NULL;
                }
                strcpy(d->fs, str);

                while (*buffer != '\n') {
                    *buffer--;
                }

                str[0] = '\0';
                for (i = 0; *buffer != ' '; i++) {
                    str[i] = *buffer++;
                }
                str[i] = '\0';

		d->part = mem_alloc(strlen(str) + 1);
                if (!d->part) {
                    free(buffer_);
                    free(d->fs);
                    free(d);
                    return NULL;
                }
                strcpy(d->part, str);

                break;

            } else {
                while (i--) {
                    buffer--;
            }
            buffer++;
        }
        free(buffer_);

        return d;
}
