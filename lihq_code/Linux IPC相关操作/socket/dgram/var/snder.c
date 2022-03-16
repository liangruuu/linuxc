#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "proto.h"

#define IPSTRSIZE 40

int main(int argc, char * argv[])
{
    int sd, size;
    struct sockaddr_in raddr;
    struct msg_st *sbufp;



    if(argc < 3)
    {
        fprintf(stderr, "Usage:%s <IP ADDRESS> <NAME>\n",argv[0]);
        exit(1);
    }

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }

    if(strlen(argv[2]) > NAMEMAX)
    {
        fprintf(stderr, "NAME is too long!\n");
        exit(1);
    }

    size = sizeof(struct msg_st) + strlen(argv[2]);
    sbufp = malloc(size);
    if(sbufp == NULL)
    {
        perror("malloc()");
        exit(1);
    }

    memset(sbufp, '\0', sizeof(*sbufp));
    strcpy(sbufp->name, argv[2]);
    sbufp->math = htonl(rand() % 100);
    sbufp->chinese = htonl(rand() % 100);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(RCVPROT));
    inet_pton(AF_INET, argv[1], &raddr.sin_addr);

    if(sendto(sd, sbufp, size, 0, (void *)&raddr, sizeof(raddr)) < 0)
    {
        perror("sendto()");
        exit(1);
    }

    puts("ok!");

    close(sd);

    exit(0);
}