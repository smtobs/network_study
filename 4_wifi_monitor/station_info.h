#ifndef STATION_INFO_H
#define STATION_INFO_H

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>
#include <net/if.h>
#include <netlink/route/link.h>
#include <linux/if_ether.h>

void station_handler(struct nlattr **sinfo);
void set_mac_addr(const unsigned char *mac_addr);
unsigned char *get_mac_addr();

#endif
