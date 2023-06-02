#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    struct sockaddr_in client_addr;
    unsigned int client_len;
    unsigned short broadcast_port = BROADCAST_PORT;
    char recv_string[MAX_BUFSIZE + 1];
    int recv_string_len;

    /* Create Socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket() failed");
        exit(1);
    }

    /* Set broadcast */
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family      = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    broadcast_addr.sin_port        = htons(broadcast_port);

    /* Bind */
    if (bind(sock, (struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr)) < 0)
    {
        perror("bind() failed");
        exit(1);
    }

    while (1)
    {
        /* Receive */
        if ((recv_string_len = recvfrom(sock, recv_string, MAX_BUFSIZE, 0, (struct sockaddr *)&client_addr, &client_len)) < 0)
        {
            perror("recvfrom() failed");
            exit(1);
        }

        recv_string[recv_string_len] = '\0';
        printf("Received Broadcast message: %s\n", recv_string);
        memset(recv_string, 0x0, recv_string_len);
    }

    /* Close Socket */
    close(sock);
    return 0;
}

