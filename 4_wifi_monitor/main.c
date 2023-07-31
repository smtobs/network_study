#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>
#include <net/if.h>
#include <netlink/route/link.h>
#include <linux/if_ether.h>

#include "callback.h"

static struct nl_sock *nl_sock;
static int nl80211_id;

extern int scan_callback(struct nl_msg *msg, void *arg);
extern int get_iface_callback(struct nl_msg *msg, void *arg);
extern int get_station_callback(struct nl_msg *msg, void *arg);

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)err - 1;
    int len = nlh->nlmsg_len;
    struct nlattr *attrs;
    struct nlattr *tb[NLMSGERR_ATTR_MAX + 1];
    int *ret = arg;
    int ack_len = sizeof(*nlh) + sizeof(int) + sizeof(*nlh);

    if (err->error > 0)
    {
        /*
         * This is illegal, per netlink(7), but not impossible (think
         * "vendor commands"). Callers really expect negative error
         * codes, so make that happen.
         */
        fprintf(stderr,
                "ERROR: received positive netlink error code %d\n",
                err->error);
        *ret = -1;
    }
    else
    {
        *ret = err->error;
    }

    if (!(nlh->nlmsg_flags & NLM_F_ACK_TLVS))
    {
        return NL_STOP;
    }

    if (!(nlh->nlmsg_flags & NLM_F_CAPPED))
    {
        ack_len += err->msg.nlmsg_len - sizeof(*nlh);
    }

    if (len <= ack_len)
    {
        return NL_STOP;
    }

    attrs = (void *)((unsigned char *)nlh + ack_len);
    len -= ack_len;

    nla_parse(tb, NLMSGERR_ATTR_MAX, attrs, len, NULL);
    if (tb[NLMSGERR_ATTR_MSG])
    {
        len = strnlen((char *)nla_data(tb[NLMSGERR_ATTR_MSG]),
                      nla_len(tb[NLMSGERR_ATTR_MSG]));
        fprintf(stderr, "kernel reports: %*s\n", len,
                (char *)nla_data(tb[NLMSGERR_ATTR_MSG]));
    }

    return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
    int *ret = arg;
    *ret = 0;
    return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
    int *ret = arg;
    *ret = 0;
    return NL_STOP;
}

int main(int argc, char *argv[])
{
    struct nl_cb *cb;
    struct nl_msg *msg;
    int err;

    if (argc < 2)
    {
        printf("You must provide the name of the WiFi interface as an argument.\n");
        return -1;
    }

    const char *interface        = argv[1];
    unsigned int interface_index = if_nametoindex(interface);

    if (interface_index == 0)
    {
        printf("Invalid interface name: %s\n", interface);
        return -1;
    }

    nl_sock = nl_socket_alloc();
    if (!nl_sock)
    {
        printf("Failed to allocate netlink socket.\n");
        return -1;
    }

    nl_socket_set_buffer_size(nl_sock, 8192, 8192);

    if (genl_connect(nl_sock))
    {
        printf("Failed to connect to generic netlink.\n");
        nl_socket_free(nl_sock);
        return -1;
    }

    nl80211_id = genl_ctrl_resolve(nl_sock, "nl80211");
    if (nl80211_id < 0)
    {
        printf("nl80211 not found.\n");
        nl_socket_free(nl_sock);
        return -1;
    }

    msg = nlmsg_alloc();
    if (!msg)
    {
        printf("Failed to allocate netlink message.\n");
        nl_socket_free(nl_sock);
        return -1;
    }

    int nl_debug = 0;
    cb = nl_cb_alloc(nl_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);

    if (!cb)
    {
        printf("Failed to allocate netlink callbacks.\n");
        nlmsg_free(msg);
        nl_socket_free(nl_sock);
        return -1;
    }

    genlmsg_put(msg, 0, 0, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, interface_index);

    err = nl_send_auto_complete(nl_sock, msg);
    if (err < 0)
    {
        goto out;
    }
    err = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, scan_callback, NULL);
    while (err > 0)
    {
        nl_recvmsgs(nl_sock, cb);
    }

    genlmsg_put(msg, 0, 0, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, interface_index);

    err = nl_send_auto_complete(nl_sock, msg);
    if (err < 0)
    {
        goto out;
    }

    err = 1;
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, get_iface_callback, NULL);
    while (err > 0)
    {
        nl_recvmsgs(nl_sock, cb);
    }
    genlmsg_put(msg, 0, 0, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);
    NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, "80:CA:4B:85:1E:E7");

    err = nl_send_auto_complete(nl_sock, msg);
    if (err < 0)
    {
        goto out;
    }

    err = 1;
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, get_station_callback, NULL);

    while (err > 0)
    {
        nl_recvmsgs(nl_sock, cb);
    }

nla_put_failure:
out:
    nlmsg_free(msg);
    nl_cb_put(cb);
    nl_socket_free(nl_sock);

    return 0;
}
