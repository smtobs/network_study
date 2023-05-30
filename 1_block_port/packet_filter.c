
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#define BLOCK_PORT 16450

static struct nf_hook_ops nfho;

unsigned int block_port_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;

    /* Check IP header */
    iph = ip_hdr(skb);
    if (iph->protocol == IPPROTO_TCP)
    {
        /* Check TCP header */
        tcph = tcp_hdr(skb);
        if (ntohs(tcph->dest) == BLOCK_PORT)
        {
            printk(KERN_INFO "Blocked TCP packet with destination port %d\n", BLOCK_PORT);
            return NF_DROP;
        }
    }

    return NF_ACCEPT;
}

static int __init block_port_init(void)
{
    printk(KERN_INFO "Initializing Block Port module\n");

    /* Set Netfilter */
    nfho.hook     = block_port_hook;
    nfho.hooknum  = NF_INET_PRE_ROUTING;
    nfho.pf       = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    /* Register for Netfilter */
    nf_register_net_hook(&init_net, &nfho);

    return 0;
}

static void __exit block_port_exit(void)
{
    printk(KERN_INFO "Exiting Block Port module\n");

    /* Remove Netfilter */
    nf_unregister_net_hook(&init_net, &nfho);
}

module_init(block_port_init);
module_exit(block_port_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bsoh");
MODULE_DESCRIPTION("Block Port Driver");

