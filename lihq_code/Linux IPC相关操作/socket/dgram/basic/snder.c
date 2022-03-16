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
    int sd;
    struct sockaddr_in raddr;
    struct msg_st sbuf;
    socklen_t raddr_len;
    char ipstr[IPSTRSIZE];

    if(argc < 2)
    {
        fprintf(stderr, "Usage:%s <IP ADDRESS>\n",argv[0]);
        exit(1);
    }

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }

    memset(&sbuf, '\0', sizeof(sbuf));
    strcpy(sbuf.name, "Alan");
    sbuf.math = htonl(rand() % 100);
    sbuf.chinese = htonl(rand() % 100);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(RCVPROT));
    inet_pton(AF_INET, argv[1], &raddr.sin_addr);

    if(sendto(sd, &sbuf, sizeof(sbuf), 0, (void *)&raddr, sizeof(raddr)) < 0)
    {
        perror("sendto()");
        exit(1);
    }

    puts("ok!");

    close(sd);

    exit(0);
}