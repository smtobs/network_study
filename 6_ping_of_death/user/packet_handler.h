#ifndef PACKET_HANDLER_H_
#define PACKET_HANDLER_H_

#include "filter_list.h"
#include <netinet/in.h>
#include <pcap.h>
#include <pthread.h>

typedef struct packet_processor
{
    filter_list *ip_filter_head;
    pthread_mutex_t lock;
    unsigned long long packet_total_size;
    unsigned char index;
    char *interface_name;
    pthread_t thread;
} packet_processor;

void packet_handler(unsigned char *user_data, const struct pcap_pkthdr *pkthdr, const unsigned char *packet);

#endif // PACKET_HANDLER_H_
