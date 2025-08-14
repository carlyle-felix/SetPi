#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../incl/manager.h"

#define MAX_BUFFER 128

struct value_list {
    char *arm_freq;
    char *gpu_freq;
    char *ov; 
};

char *get_buffer(const char *p);
void truncate(char *buffer, uint16_t pos, uint8_t t_len);
void expand(char *buffer, uint16_t pos, uint8_t t_len);
void *mem_alloc(uint16_t n);

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
            if (l->ov) {
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
    
    fp = fopen("/home/carlyle/config.txt", "r");       // /boot/firmware/config.txt", "r"
    if (fp) {
        return "/home/carlyle/config.txt";
    }

    fp = fopen("/boot/config.txt", "r");
    if (!fp) {
        printf("error: unable to locate config.txt.\n");
        return NULL;
    }

    return "/boot/config.txt";
}

int8_t write_config(List l)
{
    FILE *fp;
    register uint8_t i, j, k;
    uint8_t num_keys, key_len, len;
    const char *key[] = {"arm_freq", "gpu_freq", "over_voltage_delta"};
    char *buffer, *path, *str, value[MAX_BUFFER];

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
    buffer = realloc(buffer, strlen(buffer) + MAX_BUFFER);
    if (!buffer) {
        printf("error: failed to reallocate memory buffer in write_config().\n");
        return -1;
    }

    num_keys = (uint8_t) (sizeof(key) / sizeof(key[0]));
    str = buffer;
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

        buffer = str;
        while (*buffer) {
            // ignore comments
            if (*buffer == '#') {
                while (*buffer++ != '\n');
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
                while (*buffer++ != '\n') {
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
                    expand(str, buffer - str, (len - k));
		} else if (len < k) {
                    truncate(str, (buffer + len) - str, k - len);
     		}

                j = 0;
                while (value[j]) {
                    *buffer++ = value[j++];
                }

            } else {
                while (j--) {
                    buffer--;
                    continue;
                }
            }
            buffer++;
        }
    }

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

    free(str);  // buffer placeholder
    
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
