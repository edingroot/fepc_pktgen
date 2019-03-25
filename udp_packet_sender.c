#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int stop = 0;

static void catch_function(int signo);

int main(int argc, char **argv) {
    int sendfd;
    struct sockaddr_in destaddr;
    int block_size = 1458; // default packet size 1500
    char *pktbuf;

    // Parse args
    if (argc < 3) {
        printf("usage: udp_packet_sender <dest_ip> <dest_port> <block_size>\n");
        return 1;
    } else if (argc == 4) {
        block_size = atoi(argv[3]);
    }

    // Register signal handler
    if (signal(SIGINT, catch_function) == SIG_ERR) {
        fputs("An error occurred while setting a signal handler.\n", stderr);
        return 1;
    }

    // Allocate and fill the packet buffer
    pktbuf = malloc(block_size);
    memset(pktbuf, 1, block_size);

    // Dest addr info
    bzero(&destaddr, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &destaddr.sin_addr);

    // Create socket
    sendfd = socket(AF_INET, SOCK_DGRAM, 0);

    printf("Sending UDP packets to %s:%d with block size %d\n", argv[1], atoi(argv[2]), block_size);

    while (!stop) {
        sendto(sendfd, pktbuf, block_size, 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
    }

    free(pktbuf);
    return 0;
}

static void catch_function(int signo) {
    if (signo == SIGINT) {
        printf("Stopped by SIGINT.\n");
        stop = 1;
    }
}
