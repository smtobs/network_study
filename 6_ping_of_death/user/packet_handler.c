#include "packet_handler.h"

#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <stdio.h>

void packet_handler(unsigned char *user_data, const struct pcap_pkthdr *pkthdr, const unsigned char *packet)
{
    packet_processor *proc = (packet_processor *)user_data;
    
    /* Checking Ethernet header type. */
    struct ether_header *eth_header = (struct ether_header *)(packet);
    if (ntohs(eth_header->ether_type) != ETHERTYPE_IP)
    {
        return;
    }

    /* Checking IP header protocol */
    struct ip *ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    if (ip_header->ip_p != IPPROTO_ICMP)
    {
        return;
    }

    /* Checking ICMP header type. */
    struct icmp *icmp_header = (struct icmp *)((unsigned char *)ip_header + (ip_header->ip_hl << 2));
    if (icmp_header->icmp_type == ICMP_ECHO)
    {
        pthread_mutex_lock(&proc->lock);

        proc->icmp_packet_total_size += ntohs(ip_header->ip_len);

        if (!find_filter_list_ip(proc->ip_filter_head, &ip_header->ip_src))
        {   
            /* Adding to IP filter list. */
            add_new_filter_list_ip(&proc->ip_filter_head, &ip_header->ip_src);
            fprintf(stderr, "Added new filter list ip: %s\n", inet_ntoa(ip_header->ip_src));
        }
        pthread_mutex_unlock(&proc->lock);
    }
}
