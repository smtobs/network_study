#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "event_handler.h"

static int init_ioctl_cmd(const char *dev_path, int flags);
static void deinit_ioctl_cmd(int fd);

void init_event_handler(struct event_msg *event)
{
    struct mq_attr attr;
    attr.mq_flags = event->nonblock_flag;
    attr.mq_maxmsg = event->max_msg;
    attr.mq_msgsize = event->size;
    attr.mq_curmsgs = 0;

    event->mq = mq_open(event->dev_name, O_RDWR | O_CREAT, 0600, &attr);
    if (event->mq == (mqd_t)-1)
    {
        perror("message queue: mq_open");
        exit(1);
    }

    event->ioc_fd = init_ioctl_cmd(event->ioc_path, event->ioc_flags);
}

void event_send(struct event_msg *event)
{
    if (mq_send(event->mq, (const char *)&(event->msg), sizeof(struct msg_data), 0) == -1)
    {
        perror("Sender: mq_send");
        // exit(1);
    }
}

void event_loop(struct event_msg *event)
{
    fd_set readset;
    struct timeval timeout;
    int ret;

    while (1)
    {
        FD_ZERO(&readset);
        FD_SET(event->mq, &readset);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        ret = select(event->mq + 1, &readset, NULL, NULL, &timeout);

        if (ret == -1)
        {
            perror("select");
            break;
        }
        else if (ret == 0)
        {
            printf("Timeout, no messages received in the last 5 seconds.\n");
        }
        else
        {
            if (FD_ISSET(event->mq, &readset))
            {
                struct msg_data rev_msg;
                ssize_t bytes_read = mq_receive(event->mq, (char *)&rev_msg, sizeof(struct msg_data), NULL);

                if (bytes_read >= 0)
                {
                    printf("Received: idx : [%u]\n", rev_msg.idx);
                    printf("Received: cmd : [%u]\n", rev_msg.cmd);

                    int cmd = rev_msg.cmd;
                    if (cmd < MAX_ACT_CMD)
                    {
                        event->action[cmd](&rev_msg);
                    }
                }
                else
                {
                    if (errno != EAGAIN)
                    {
                        perror("mq_receive");
                        usleep(100000);
                    }
                }
            }
        }
    }
}

void deinit_event_handler(struct event_msg *event)
{
    mq_close(event->mq);

    deinit_ioctl_cmd(event->ioc_fd);
}

static int init_ioctl_cmd(const char *dev_path, int flags)
{
    int fd = open(dev_path, flags);
    if (fd < 0)
    {
        perror("Failed to open device");
        exit(1);
    }

    return fd;
}

static void deinit_ioctl_cmd(int fd)
{
    close(fd);
}
