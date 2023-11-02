#include <linux/interrupt.h>
#include <linux/atomic.h>

#include "icmp_filter.h"

#define ACTIVE_NETFILTER     1
#define STANDBY_NETFILTER    0

static LIST_HEAD(filter_list);
static DEFINE_SPINLOCK(filter_lock);

static int major_number;
static struct cdev cdev;
static struct class *dev_class;
static struct device *dev_device;
static atomic_t is_active_netfilter = ATOMIC_INIT(0);

static struct filter_entry
{
    __be32 ip_address;     // Big endian format of IP address
    unsigned long timeout; // Timeout in jiffies
    struct list_head list; // Kernel's list structure
};

static struct nf_hook_ops nfho =
{
    .hook = icmp_filter,
    .pf = PF_INET,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
};

/*
 * icmp_ip_filter - Check if the given source IP address is in the filter list
 */
static inline int drop_icmp_pkt(__be32 src_ip)
{
    struct filter_entry *entry, *tmp;
    int pkt_status = NF_ACCEPT;
    unsigned long flags;

    spin_lock_irqsave(&filter_lock, flags);

    list_for_each_entry_safe(entry, tmp, &filter_list, list)
    {
        /* Check for entries that have timed out and remove them from the list */
        if (time_after(jiffies, entry->timeout + 60 * HZ))
        {
            list_del(&entry->list);
            kfree(entry);
        }
        else if ((htonl(entry->ip_address)) == src_ip)
        {
            printk("Drop ICMP Request Source IP Address: %pI4 \n", &src_ip);
            pkt_status = NF_DROP;
            break;
        }
    }

    spin_unlock_irqrestore(&filter_lock, flags);

    return pkt_status;
}

unsigned int icmp_filter(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    int pkt_status = NF_ACCEPT;

    if (atomic_read(&is_active_netfilter) == STANDBY_NETFILTER)
    {
        return pkt_status;
    }

    iph = ip_hdr(skb);
    if (iph->protocol == IPPROTO_ICMP)
    {
        pkt_status = drop_icmp_pkt(iph->saddr);

        /* Check empty list */
        if (list_empty(&filter_list))
        {
            if (atomic_read(&is_active_netfilter) == ACTIVE_NETFILTER)
            {
                atomic_set(&is_active_netfilter, STANDBY_NETFILTER);
            }
        }
    }
    return pkt_status;
}

static void clear_filter_list(void)
{
    struct filter_entry *entry, *tmp;

    list_for_each_entry_safe(entry, tmp, &filter_list, list)
    {
        list_del(&entry->list);
        kfree(entry);
    }
}

static bool add_to_filter(__be32 ip)
{
    struct filter_entry *new_entry;
    new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if (!new_entry)
    {
        printk(KERN_ERR "Failed to allocate memory for filter entry\n");
        return false;
    }
    new_entry->ip_address = ip;
    new_entry->timeout = jiffies;

    list_add(&new_entry->list, &filter_list);

    return true;
}

static bool is_valid_ip_address(__be32 ipv4_addr)
{
    if (ipv4_addr == 0 || ipv4_addr == 0xFFFFFFFF)
    {
        printk("Invalid IP address passed.\n");
        return false;
    }

    if ((ipv4_addr & 0xFF000000) == 0x7F000000)
    {
        printk("Loopback IP address is not allowed.\n");
        return false;
    }
    return true;
}

struct file_operations icmp_filter_fops =
{
    .unlocked_ioctl = ioctl_icmp_filter_fops,
};

static long ioctl_icmp_filter_fops(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned long flags;
    __be32 addr;

    if (_IOC_NR(cmd) >= IOC_ICMP_FILTER_MAX_CMD_NUM)
    {
        printk(KERN_ERR "ioctl : command number out of range\n");
        return -ENOTTY;
    }

    switch (cmd)
    {
    case IOCTL_CMD_ICMP_FILTER_REQ:
        if (copy_from_user(&ioctl_icmp_filter, (struct ioctl_icmp_filter __user *)arg, sizeof(ioctl_icmp_filter)))
        {
            printk(KERN_ERR "ioctl : copy_from_user failed\n");
            return -EFAULT;
        }

        if (!ioctl_icmp_filter.ip)
        {
            printk(KERN_ERR "ioctl : Filter IP address NULL.\n");
            return -EINVAL;
        }

        addr = htonl(ioctl_icmp_filter.ip);
        if (is_valid_ip_address(addr) == false)
        {
            return -EINVAL;
        }

        /* Add IP for filter list */
        spin_lock_irqsave(&filter_lock, flags);
        if (add_to_filter(addr))
        {
            atomic_set(&is_active_netfilter, ACTIVE_NETFILTER);
        }
        spin_unlock_irqrestore(&filter_lock, flags);
        break;

    default:
        return -ENOTTY;
    }
    return 0;
}

static int __init icmp_filter_init(void)
{
    dev_t dev_no;

    int ret = nf_register_net_hook(&init_net, &nfho);
    if (ret)
    {
        printk(KERN_ERR "Netfilter hook registration failed with return value %d\n", ret);
        return ret; // 모듈 로딩 실패 처리
    }

    /* Alloc Major, Minor */
    if (alloc_chrdev_region(&dev_no, 0, 1, "icmp_filter") < 0)
    {
        return -1;
    }
    major_number = MAJOR(dev_no);

    /* Register for file_operations */
    cdev_init(&cdev, &icmp_filter_fops);
    if (cdev_add(&cdev, dev_no, 1) < 0)
    {
        unregister_chrdev_region(dev_no, 1);
        return -1;
    }

    /* Create class device */
    dev_class = class_create(THIS_MODULE, "icmp_class");
    if (IS_ERR(dev_class))
    {
        cdev_del(&cdev);
        unregister_chrdev_region(dev_no, 1);
        return PTR_ERR(dev_class);
    }

    /* Create Device File */
    dev_device = device_create(dev_class, NULL, dev_no, NULL, ICMP_FILTER_DEVICE_NAME);
    if (IS_ERR(dev_device))
    {
        class_destroy(dev_class);
        cdev_del(&cdev);
        unregister_chrdev_region(dev_no, 1);
        return PTR_ERR(dev_device);
    }

    return 0;
}

static void __exit icmp_filter_exit(void)
{
    device_destroy(dev_class, MKDEV(major_number, 0));
    class_destroy(dev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(MKDEV(major_number, 0), 1);
    nf_unregister_net_hook(&init_net, &nfho);
    clear_filter_list();
}

module_init(icmp_filter_init);
module_exit(icmp_filter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bsoh");
MODULE_DESCRIPTION("ICMP Filter Driver");
