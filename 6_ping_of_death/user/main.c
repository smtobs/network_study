#include <stdio.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/ethernet.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/ioctl.h>

#include "event_handler.h"
#include "packet_handler.h"
#include "filter_list.h"
#include "../ioctl/ioctl_icmp_filter.h"

#define MONITORING_INTERVAL 10 // seconds
#define SIZE_THRESHOLD 8000    // bytes

extern void packet_handler(unsigned char *user_data, const struct pcap_pkthdr *pkthdr, const unsigned char *packet);

struct event_msg event;
struct packet_processor *ping_monitor = NULL;
int num_interfaces;

void alarm_handler(int signo)
{
    if (signo == SIGALRM)
    {
        for (int i = 0; i < num_interfaces; i++)
        {
            pthread_mutex_lock(&ping_monitor[i].lock);

            /* Checking icmp pacekt total size. */
            if (ping_monitor[i].icmp_packet_total_size > SIZE_THRESHOLD)
            {
                /* Event sending to the event loop. */
                event.msg.cmd = IOCTL_ACT_FILTER_RUL_REQ_CMD;
                event.msg.idx = i;
                event_send(&event);
            }
            else
            {
                clear_all_filter_list_ip(&ping_monitor[i].ip_filter_head);
            }
            fprintf(stderr, "icmp packet total size : %llu\n", ping_monitor[i].icmp_packet_total_size);
            ping_monitor[i].icmp_packet_total_size = 0;

            pthread_mutex_unlock(&ping_monitor[i].lock);
        }
        alarm(MONITORING_INTERVAL);
    }
}

void ioctl_filter_rul_req(struct msg_data *msg)
{
    unsigned char idx = msg->idx;

    pthread_mutex_lock(&ping_monitor[idx].lock);

    filter_list *current = ping_monitor[idx].ip_filter_head;;
    while (current != NULL)
    {
        struct _ioctl_icmp_filter filter;

        filter.ip = current->ip.s_addr;
        filter.policy = ICMP_PACKET_DROP;

        ioctl(event.ioc_fd, IOCTL_CMD_ICMP_FILTER_REQ, &filter);
        current = current->next;
    }

    clear_all_filter_list_ip(&ping_monitor[idx].ip_filter_head);

    pthread_mutex_unlock(&ping_monitor[idx].lock);
}

void *monitor_interface_thread_func(void *data)
{
    struct packet_processor *monitor = (struct packet_processor *)data;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    struct bpf_program fp;
    char filter_exp[] = "icmp[icmptype] = icmp-echo";

    printf("Initializing monitoring for interface: %s\n", monitor->interface_name);

    handle = pcap_open_live(monitor->interface_name, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL)
    {
        fprintf(stderr, "Could not open device %s: %s\n", monitor->interface_name, errbuf);
        return NULL;
    }

    /* Compile and apply the filter */
    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1)
    {
        fprintf(stderr, "Could not parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return NULL;
    }

    if (pcap_setfilter(handle, &fp) == -1)
    {
        fprintf(stderr, "Could not install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return NULL;
    }

    /* Run */
    if (pcap_loop(handle, 0, packet_handler, (void *)monitor) < 0)
    {
    	fprintf(stderr, "pcap_loop() failed: %s\n", pcap_geterr(handle));
    }
    pcap_close(handle);
    return NULL;
}

void *event_handler_thread_func(void *arg)
{
    event.dev_name = "/my_msg_queue";
    event.nonblock_flag = 1;
    event.max_msg = 10;
    event.size = sizeof(struct msg_data);
    snprintf(event.ioc_path, sizeof(event.ioc_path), "/dev/%s", ICMP_FILTER_DEVICE_NAME);
    event.ioc_flags = O_RDWR;
    event.action[IOCTL_ACT_FILTER_RUL_REQ_CMD] = ioctl_filter_rul_req;

    /* Init Message Queue */
    init_event_handler(&event);

    /* Run Event Loop */
    event_loop(&event);

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t event_handler_thread;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <interface1> <interface2>... \n", argv[0]);
        return 1;
    }

    num_interfaces = argc - 1;

    /* Allocate memory for the ping_monitor structures. */
    ping_monitor = malloc(num_interfaces * sizeof(packet_processor));
    if (!ping_monitor)
    {
        perror("Faisled to allocate memory");
        return 1;
    }

    signal(SIGALRM, alarm_handler);
    alarm(MONITORING_INTERVAL);

    /* Create network monitoring thread */
    for (int i = 0; i < num_interfaces; i++)
    {
        ping_monitor[i].index = i;
        ping_monitor[i].interface_name = argv[i + 1];
        pthread_mutex_init(&ping_monitor[i].lock, NULL);
        pthread_create(&ping_monitor[i].thread, NULL, monitor_interface_thread_func, &ping_monitor[i]);
    }

    /* Create Event Handler thread */
    if (pthread_create(&event_handler_thread, NULL, event_handler_thread_func, NULL) != 0)
    {
        perror("Failed to create receiver thread");
        exit(1);
    }

    for (int i = 0; i < num_interfaces; i++)
    {
        pthread_join(ping_monitor[i].thread, NULL);
    }

    pthread_join(event_handler_thread, NULL);

    deinit_event_handler(&event);

    for (int i = 0; i < num_interfaces; i++)
    {
    	clear_all_filter_list_ip(&ping_monitor[i].ip_filter_head);
    }
    free(ping_monitor);
    return 0;
}
