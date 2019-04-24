#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>

#include "udp.h"

#define MAX_ETH_LEN 9700
#define MAX_DATA_SIZE 9600

int stop = 0;

static void catch_function(int signo);

int main(int argc, char **argv)
{
    int raw_sock;
    uint8_t packet[MAX_ETH_LEN];
    uint8_t udp_packet[MAX_ETH_LEN];
    uint8_t data[MAX_DATA_SIZE];
    char *sending_data;
    unsigned int packet_size;
    unsigned int data_size;
    struct sockaddr_in src_addr;
    struct sockaddr_in dst_addr;

    // Parse args
    if (argc < 5)
    {
        printf("Usages:\n"
               "  udp_packet_sender <src_ip> <dest_ip> <dest_port> <data_size>\n"
               "  udp_packet_sender - <dest_ip> <dest_port> <data_size>\n");
        return 1;
    }

    // Register signal handler
    if (signal(SIGINT, catch_function) == SIG_ERR) {
        fputs("An error occurred while setting a signal handler.\n", stderr);
        return 1;
    }

    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(2048);
    if (argv[1][0] != '-')
        inet_aton(argv[1], &src_addr.sin_addr);

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(atoi(argv[3]));
    inet_aton(argv[2], &dst_addr.sin_addr);

    // Allocate and fill the packet buffer
    data_size = atoi(argv[4]);
    sending_data = malloc(data_size);
    memset(sending_data, 1, data_size);

    printf("[+] Build UDP packet...\n");
    packet_size = build_udp_packet(src_addr, dst_addr, 
                                   udp_packet + sizeof(struct iphdr), data, data_size);

    printf("[+] Build IP packet...\n");
    packet_size = build_ip_packet(src_addr.sin_addr, dst_addr.sin_addr,
                                  IPPROTO_UDP, udp_packet, udp_packet + sizeof(struct iphdr), packet_size);

    // Create a raw socket
    if ((raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        perror("socket");
        exit(1);
    }

    printf("Sending UDP packets to %s:%d with data size %d, packet size %d\n",
           argv[2], atoi(argv[3]), data_size, packet_size);
    while (!stop)
    {
        printf("[+] Send UDP packet...\n");
        if (sendto(raw_sock, udp_packet, packet_size, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0)
        {
            perror("sendto");
            exit(1);
        }
    }

    free(sending_data);
    return 0;
}

static void catch_function(int signo)
{
    if (signo == SIGINT)
    {
        printf("\nStopped by SIGINT.\n");
        stop = 1;
    }
}
