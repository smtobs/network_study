#include <stdlib.h>
#include <stdio.h>

#include "filter_list.h"

void clear_all_filter_list_ip(filter_list **head)
{
    filter_list *current = *head;
    while (current)
    {
        filter_list *to_delete = current;
        current = current->next;
        free(to_delete);
    }
    *head = NULL;
}

filter_list *find_filter_list_ip(filter_list *head, struct in_addr *ip)
{
    filter_list *current = head;
    while (current)
    {
        if (current->ip.s_addr == ip->s_addr)
        {
            return current;
        }
        current = current->next;
    }
    return NULL; // If IP not found in list
}

void add_new_filter_list_ip(filter_list **head, struct in_addr *ip)
{
    filter_list *new_node = malloc(sizeof(filter_list));
    new_node->ip = *ip;
    new_node->next = *head;
    *head = new_node;
}
