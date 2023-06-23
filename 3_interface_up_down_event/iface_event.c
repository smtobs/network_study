#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/if.h>

#define MAX_MSG_BUFFER     4096
#define FAIL               (-1)

static int init_netlink_socket(struct sockaddr_nl *sa)
{
    int rtnl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (rtnl_socket < 0)
    {
        printf("Error: could not open rtnetlink socket\n");
        return FAIL;
    }

    memset(sa, 0, sizeof(*sa));
    sa->nl_family = AF_NETLINK;
    sa->nl_groups = RTMGRP_LINK;

    if (bind(rtnl_socket, (struct sockaddr*) sa, sizeof(*sa)) < 0)
    {
        printf("Error: could not bind rtnetlink socket\n");
        close(rtnl_socket);
        return FAIL;
    }

    return rtnl_socket;
}

static ssize_t read_netlink_msg(int sock, struct msghdr *msg)
{
    memset(msg, 0x0, sizeof(*msg));

    char buffer[MAX_MSG_BUFFER];
    struct iovec iov = {buffer, sizeof(buffer)};
    struct sockaddr_nl snl;
    msg->msg_name    = (void*)&snl;
    msg->msg_namelen = sizeof(snl);
    msg->msg_iov     = &iov;
    msg->msg_iovlen  = 1;

    ssize_t len = recvmsg(sock, msg, 0);
    if (len < 0)
    {
        printf("Error: could not read netlink message\n");
    }

    return len;
}

static void netlink_msg_processing(struct msghdr *msg, ssize_t len)
{
    struct nlmsghdr *nh;

    for (nh = (struct nlmsghdr *)msg->msg_iov->iov_base; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len))
    {
        if (nh->nlmsg_type == NLMSG_DONE)
        {
            break;
        }

        if (nh->nlmsg_type == RTM_NEWLINK || nh->nlmsg_type == RTM_DELLINK)
        {
            struct ifinfomsg *iface = NLMSG_DATA(nh);
            struct rtattr *hdr = IFLA_RTA(iface);
            int remains = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*iface));

            /* Get interface name */
            char ifname[IFNAMSIZ] = {0};
            while (RTA_OK(hdr, remains))
            {
                if (hdr->rta_type == IFLA_IFNAME)
                {
                    strncpy(ifname, RTA_DATA(hdr), IFNAMSIZ - 1);
                    break;
                }
                hdr = RTA_NEXT(hdr, remains);
            }

            printf("Interface %s is %s\n", ifname, (iface->ifi_flags & IFF_UP) ? "up" : "down");
        }
    }
}

int main(void)
{
    struct sockaddr_nl sa;
    int rtnl_socket = init_netlink_socket(&sa);
    if (rtnl_socket < 0)
    {
        return -1;
    }

    while (1)
    {
        struct msghdr msg;
        ssize_t len = read_netlink_msg(rtnl_socket, &msg);
        if (len < 0) 
        {
            break;
        }

        netlink_msg_processing(&msg, len);
    }

    close(rtnl_socket);
    return 0;
}
