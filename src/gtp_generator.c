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

#define RECV_BUF 90000

typedef struct gtp_header_t {
    uint8_t flag;
    uint8_t type;
    uint16_t length;
    uint32_t teid;
} __attribute__((packed)) gtp_header;

struct ip {
#if __BYTE_ORDER__ == LITTLE_ENDIAN
    u_char ip_hl : 4, /* header length */
        ip_v : 4;     /* version */
#endif
#if __BYTE_ORDER__ == BIG_ENDIAN
    u_char ip_v : 4, /* version */
        ip_hl : 4;   /* header length */
#endif
    u_char ip_tos;                 /* type of service */
    short ip_len;                  /* total length */
    u_short ip_id;                 /* identification */
    short ip_off;                  /* fragment offset field */
#define IP_DF 0x4000               /* dont fragment flag */
#define IP_MF 0x2000               /* more fragments flag */
    u_char ip_ttl;                 /* time to live */
    u_char ip_p;                   /* protocol */
    u_short ip_sum;                /* checksum */
    struct in_addr ip_src, ip_dst; /* source and dest address */
};

typedef struct ip_h {
    uint8_t version;
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t proto;
    uint16_t chksum;
    uint32_t src;
    uint32_t dst;

} IP;

uint16_t calculate_ip_chksum(IP *ip) {
    uint16_t *buf = (uint16_t *)ip;
    uint32_t sum = 0;
    uint16_t checksum = 0;

    int counter = (ip->version & 0xF) * 4;
    while (counter > 1) {
        sum += *buf++;
        counter -= 2;
    }

    if (counter == 1)
        sum += *(uint8_t *)buf;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    checksum = ~sum;

    return checksum;
}

int main(int argc, char **argv) {
    int rawfd, udpfd, gtpfd, n, rv;
    struct sockaddr_in servaddr, remoteaddr;
    struct in_addr user_src_ip, user_dst_ip;
    char recvbuffer[RECV_BUF];
    char sendbuffer[RECV_BUF + sizeof(gtp_header)];

    if (argc != 6) {
        printf("Usage: gtp_generator <udp_listener_port> <gtp_dst_ip> <up_src_ip> <up_dst_ip> <gtp_teid>\n");
        return -1;
    }

    // Socket for RX (raw socket server)
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    rawfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (bind(rawfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("An error occurred while binding raw socket server, errno=%d\n", errno);
        printf("Root permission is needed for binding raw socket.\n");
        return 1;
    }

    // Socket for RX (udp socket server) for preventing ICMP Port Unreachable reply
    servaddr.sin_port = htons(atoi(argv[1]));
    udpfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("An error occurred while binding udp server, errno=%d, skip.\n", errno);
    }

    // Socket for TX (gtp sender)
    bzero(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_port = htons(2152);
    inet_pton(AF_INET, argv[2], &remoteaddr.sin_addr);
    gtpfd = socket(AF_INET, SOCK_DGRAM, 0);
    // printf("%d\n", setsockopt(gtpfd, SOL_SOCKET, SO_BINDTODEVICE, "enp0s9", 6));

    // User plane ip header
    inet_aton(argv[3], &user_src_ip);
    inet_aton(argv[4], &user_dst_ip);

    // Connect sender datagram socket to destination address
    if ((rv = connect(gtpfd, (struct sockaddr *) &remoteaddr, sizeof(remoteaddr))) != 0) {
        fprintf(stderr, "connect: %s\n", gai_strerror(rv));
        close(gtpfd);
        return 0;
    }

    // Static GTP header
    gtp_header gtpuheader;
    gtpuheader.flag = 0x30;
    gtpuheader.type = 255;
    gtpuheader.teid = htonl(atol(argv[5]));

    printf("[INFO] UDP packet listener listening on port %d, recv buffer size = %d\n", atoi(argv[1]), RECV_BUF);
    printf("[INFO] GTP destination IP = %s\n", argv[2]);
    printf("[INFO] User plane source IP = %s\n", argv[3]);
    printf("[INFO] User plane destination IP = %s\n", argv[4]);
    printf("[INFO] GTP TEID = %s\n", argv[5]);

    while (1) {
        n = recvfrom(rawfd, recvbuffer, RECV_BUF, 0, NULL, NULL);
        // printf("Received pkt size: %d\n", n);
        gtpuheader.length = htons(n + sizeof(gtp_header));

        struct ip *ip_header = (struct ip *)recvbuffer;
        // printf("Before modification: %s\n", (inet_ntoa(ip_header->ip_dst)));
        // inet_aton("", &ip_header->ip_src.s_addr);
        // printf("SRC Address: %s\n\n", (inet_ntoa(ip_header->ip_src)));

        memcpy(&ip_header->ip_src.s_addr, &user_src_ip.s_addr, sizeof(user_src_ip.s_addr));
        memcpy(&ip_header->ip_dst.s_addr, &user_dst_ip.s_addr, sizeof(user_dst_ip.s_addr));
        // printf("After modification: %s\n\n", (inet_ntoa(ip_header->ip_dst)));
        // printf("strlen(recvbuffer) = %d\n", n);
        // printf("IHL = %d\n", ip_header->ip_len);

        ip_header->ip_sum = 0;
        ip_header->ip_sum = calculate_ip_chksum((IP *)recvbuffer);
        // printf("ip_checksum = %x\n",ip_header->ip_sum);

        memcpy(sendbuffer, &gtpuheader, sizeof(gtp_header));
        memcpy(sendbuffer + sizeof(gtp_header), recvbuffer, n);

        // sendto(gtpfd, sendbuffer, n + sizeof(gtp_header), 0, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
        write(gtpfd, sendbuffer, n + sizeof(gtp_header));
    }

    close(rawfd);
    close(udpfd);
    close(gtpfd);
    return 0;
}
