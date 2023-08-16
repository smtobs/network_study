#ifndef FILTER_LIST_H
#define FILTER_LIST_H

#include <netinet/in.h>

typedef struct filter_list
{
    struct in_addr ip;
    struct filter_list *next;
} filter_list;

void clear_all_ip(filter_list **head);
filter_list *find_ip(filter_list *head, struct in_addr *ip);
void add_new_ip(filter_list **head, struct in_addr *ip);

#endif // FILTER_LIST_H
