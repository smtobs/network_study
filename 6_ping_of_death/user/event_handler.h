#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
	NULL_ACT_CMD = 0,
	IOCTL_ACT_CMD,
	MAX_ACT_CMD,
};

struct msg_data
{
	unsigned char cmd;
	unsigned char idx;
};

typedef void (*action_func)(struct msg_data *);

struct event_msg
{
	long nonblock_flag;
	char *dev_name;
	mqd_t mq;
	long max_msg;
	long size;
	struct msg_data msg;
	int ioc_fd;
	char ioc_path[32];
	int ioc_flags;
	action_func action[MAX_ACT_CMD];
};

void init_event_handler(struct event_msg *);
void event_send(struct event_msg *);
void event_loop(struct event_msg *);
void deinit_event_handler(struct event_msg *);
#endif


