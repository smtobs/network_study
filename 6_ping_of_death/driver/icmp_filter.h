#ifndef _ICMP_FILTER_H_
#define _ICMP_FILTER_H_

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#include "../ioctl/ioctl_icmp_filter.h"

static int drop_icmp_pkt(__be32);
static unsigned int icmp_filter(void *, struct sk_buff *, const struct nf_hook_state *);
static void clear_filter_list(void);
static bool add_to_filter(__be32);
static bool is_valid_ip_address(__be32);
static long ioctl_icmp_filter_fops(struct file *, unsigned int, unsigned long);

#endif // _ICMP_FILTER_H_

