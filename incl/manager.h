#ifndef MANAGER_H
#define MANAGER_H

#include <stdint.h>

typedef struct node *List;

List create_list(void);
void delete_list(List l);
List add_item(List l, const char *item);
int8_t set_values(List l);
List get_values(List l);
int8_t save_profile(char *str);
int8_t new_profile(List l, char *str);
int8_t delete_profile(char *str);
int8_t apply_profile(char *str);
int8_t profile_list(void);
char *current_profile(void);
void print_list(List l);

#endif
