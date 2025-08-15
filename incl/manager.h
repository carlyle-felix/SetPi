#ifndef MANAGER_H
#define MANAGER_H

#include <stdint.h>

typedef enum key {ARM, GPU, OV} Key;
typedef struct value_list *List;

List create_list(void);
void delete_list(List l);
int8_t set_value(List l, Key k, const char *value);
int8_t write_config(List l);
char *current_value(Key k);

#endif
