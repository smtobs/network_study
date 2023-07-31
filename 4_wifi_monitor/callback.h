#ifndef CALLBACK_H
#define CALLBACK_H

#include "station_info.h"

int scan_callback(struct nl_msg *msg, void *arg);
int get_iface_callback(struct nl_msg *msg, void *arg);
int get_station_callback(struct nl_msg *msg, void *arg);
#endif
