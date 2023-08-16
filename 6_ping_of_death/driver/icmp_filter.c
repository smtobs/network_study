#include "icmp_filter.h"

static LIST_HEAD(filter_list);
static DEFINE_SPINLOCK(filter_lock);

static int major_number;
static struct cdev cdev;
static struct class *dev_class;
static struct device *dev_device;
static bool is_run_net_hook;

static struct filter_entry
{
    __be32 ip_address;           // Big endian format of IP address
    unsigned long timeout;       // Timeout in jiffies
    struct list_head list;       // Kernel's list structure
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
static inline bool icmp_ip_filter(__be32 src_ip)
{
    struct filter_entry *entry, *tmp;
    bool drop_packet = false;

    spin_lock(&filter_lock);

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
            drop_packet = true;
            break;
        }
    }

    /* Check empty list */
    if (list_empty(&filter_list))
    {
        printk("The filter list is empty.\n");
        if (is_run_net_hook == true)
        {
            nf_unregister_net_hook(&init_net, &nfho);
            is_run_net_hook = false;
        }
    }

    spin_unlock(&filter_lock);

    return drop_packet;
}

unsigned int icmp_filter(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph = ip_hdr(skb);
    if (iph->protocol == IPPROTO_ICMP && icmp_ip_filter(iph->saddr))
    {
        return NF_DROP;
    }
    return NF_ACCEPT;
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

static void add_to_filter(__be32 ip)
{
    struct filter_entry *new_entry;
    new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if (!new_entry)
    {
        printk(KERN_ERR "Failed to allocate memory for filter entry\n");
        return;
    }
    new_entry->ip_address = ip;
    new_entry->timeout = jiffies;

    list_add(&new_entry->list, &filter_list);
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
    __be32 addr;
    switch (cmd)
    {
    case IOCTL_CMD_ICMP_FILTER_REQ:

        if (!copy_from_user(&ioctl_icmp_filter, (struct ioctl_icmp_filter __user *)arg, sizeof(ioctl_icmp_filter)))
        {
            if (ioctl_icmp_filter.ip != NULL)
            {
                addr = htonl(ioctl_icmp_filter.ip);
                if (is_valid_ip_address(addr) == false)
                {
                    return -EINVAL;
                }

                spin_lock(&filter_lock);
                if (is_run_net_hook == false)
                {
                    nf_register_net_hook(&init_net, &nfho);
                    is_run_net_hook = true;
                }
                /* Add IP for filter list */
                add_to_filter(addr);
                spin_unlock(&filter_lock);
            }
            else
            {
                printk("Filter IP address NULL.\n");
                return -EINVAL;
            }
        }
        else
        {
            printk("ioctl : IOCTL_CMD_ICMP_FILTER_REQ Failed !\n");
            return -EFAULT;
        }
        break;
    default:
        return -ENOTTY;
    }
    return 0;
}

static int __init icmp_filter_init(void)
{
    dev_t dev_no;

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

    spin_lock(&filter_lock);
    if (is_run_net_hook == true)
    {
        nf_unregister_net_hook(&init_net, &nfho);
    }
    clear_filter_list();
    spin_unlock(&filter_lock);
}

module_init(icmp_filter_init);
module_exit(icmp_filter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bsoh");
MODULE_DESCRIPTION("ICMP Filter Driver");
