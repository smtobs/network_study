#include "packet_handler.h"

#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <stdio.h>

void packet_handler(unsigned char *user_data, const struct pcap_pkthdr *pkthdr, const unsigned char *packet)
{
    packet_processor *proc = (packet_processor *)user_data;
    

	    printf("zz\n");
    struct ether_header *eth_header = (struct ether_header *)(packet);
    if (ntohs(eth_header->ether_type) != ETHERTYPE_IP)
    {
	    printf("11\n");
        return;
    }

    struct ip *ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    if (ip_header->ip_p != IPPROTO_ICMP)
    {
	    printf("21\n");
        return;
    }

    struct icmp *icmp_header = (struct icmp *)((unsigned char *)ip_header + (ip_header->ip_hl << 2));

    if (icmp_header->icmp_type == ICMP_ECHO)
    {
        pthread_mutex_lock(&proc->lock);

        proc->packet_total_size += ntohs(ip_header->ip_len);

        if (!find_ip(proc->ip_filter_head, &ip_header->ip_src))
        {
            add_new_ip(&proc->ip_filter_head, &ip_header->ip_src);
            printf("Added new filter list ip: %s\n", inet_ntoa(ip_header->ip_src));
        }
        pthread_mutex_unlock(&proc->lock);
    }

    printf("zz end\n");
}
