/* broadcast send example */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "broadcast.h"

int main()
{
    int sock;
    struct sockaddr_in broadcast_addr;
    char *broadcast_ip            = "255.255.255.255";
    unsigned short broadcast_port = BROADCAST_PORT;
    char *send_string             = "Hello, Broadcast!";
    int broadcast_enable          = 1;

    /* Create Socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket() failed");
        exit(1);
    }

    /* Set broadcast */
    broadcast_enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcast_enable, sizeof(broadcast_enable)) < 0)
    {
        perror("setsockopt() failed");
        exit(1);
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));   
    broadcast_addr.sin_family      = AF_INET;                 
    broadcast_addr.sin_addr.s_addr = inet_addr(broadcast_ip);
    broadcast_addr.sin_port        = htons(broadcast_port);     

    /* Send pacekt */
    if (sendto(sock, send_string, strlen(send_string), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) != strlen(send_string))
    {
        perror("sendto() sent a different number of bytes than expected");
        exit(1);
    }

    /* Close socket */
    close(sock);
    return 0;
}
