#ifndef MANAGER_H
#define MANAGER_H

#include <stdint.h>

typedef struct node *List;

List create_list(void);
void delete_list(List l);
List add_item(List l, const char *item);
int8_t write_config(List l);
List get_values(List l);
void print_list(List l);

#endif
