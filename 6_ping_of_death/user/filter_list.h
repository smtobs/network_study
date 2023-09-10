#ifndef FILTER_LIST_H
#define FILTER_LIST_H

#include <netinet/in.h>

#define ICMP_PACKET_DROP     1

typedef struct filter_list
{
    struct in_addr ip;
    struct filter_list *next;
} filter_list;

void clear_all_filter_list_ip(filter_list **head);
filter_list *find_filter_list_ip(filter_list *head, struct in_addr *ip);
void add_new_filter_list_ip(filter_list **head, struct in_addr *ip);

#endif // FILTER_LIST_H
