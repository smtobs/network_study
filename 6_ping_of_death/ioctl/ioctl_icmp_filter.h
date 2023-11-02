#ifndef _IOCTL_ICMP_FILTER_H_
#define _IOCTL_ICMP_FILTER_H_

#include <linux/ioctl.h>
#include <linux/types.h>

#define ICMP_FILTER_DEVICE_NAME "icmp_filter"

#define IOCTL_ICMP_FILTER_MAGIC 'f'

enum IOC_FILTER_ENUM
{
    IOC_ICMP_FILTER_REQ,
    IOC_ICMP_FILTER_MAX_CMD_NUM
};

#define IOCTL_CMD_ICMP_FILTER_REQ _IOW(IOCTL_ICMP_FILTER_MAGIC, IOC_ICMP_FILTER_REQ, struct _ioctl_icmp_filter)

struct _ioctl_icmp_filter
{
    __u32 ip;
    int policy;
};

struct _ioctl_icmp_filter ioctl_icmp_filter;

#endif
