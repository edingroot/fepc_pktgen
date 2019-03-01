#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>
#include	<ifaddrs.h>
#include	<stdint.h>
typedef struct gtp_header_t
{
	uint8_t flag;
	uint8_t type;
	uint16_t length;
	uint32_t teid;
}__attribute__((packed)) gtp_header;

struct ip {
#if __BYTE_ORDER__ == LITTLE_ENDIAN 
	u_char	ip_hl:4,		/* header length */
		ip_v:4;			/* version */
#endif
#if __BYTE_ORDER__ == BIG_ENDIAN 
	u_char	ip_v:4,			/* version */
		ip_hl:4;		/* header length */
#endif
	u_char	ip_tos;			/* type of service */
	short	ip_len;			/* total length */
	u_short	ip_id;			/* identification */
	short	ip_off;			/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

int main(int argc, char **argv)
{
	int sockfd,udpfd,n;
	
	struct sockaddr_in servaddr,remoteaddr;
	char line[1500];
	void *sendbuffer,*recvbuffer;
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(5202);

	bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	if (argc != 4)
	{
		printf("usage: gtp_generator <GTP_Header_Server IP> <Destination IP> <TEID>\n");
		return -1;
	}

	for(;;)
	{
		recvbuffer = calloc(2000,sizeof(char));
		bzero(&line, sizeof(line));
		printf("[INFO] GTP Header Server IP = %s\n",argv[1]);
		printf("[INFO] Dest. IP = %s\n",argv[2]);
		printf("[INFO] TEID = %s\n",argv[3]);
		printf("Waiting for data...\n");
		n = recvfrom(sockfd, recvbuffer,2000, 0, NULL, NULL);
		printf("Received pkt size: %d\n",n);

		struct ip* ip_header = (struct ip*) recvbuffer;
		printf("Before modification: %s\n",(inet_ntoa(ip_header->ip_dst)));

		inet_aton(argv[2],&ip_header->ip_dst.s_addr);
		printf("After modification: %s\n\n",(inet_ntoa(ip_header->ip_dst)));

		sendbuffer = calloc(2000,sizeof(char));
		gtp_header* gtpuheader = calloc(1,sizeof(gtp_header));
		gtpuheader->flag = 0x30;
		gtpuheader->type = 255;
		gtpuheader->length = htons(n+sizeof(gtp_header));
		gtpuheader->teid = htonl(atol(argv[3]));
		memcpy(sendbuffer,gtpuheader, sizeof(gtp_header));		
		memcpy(sendbuffer+sizeof(gtp_header), recvbuffer, n);
			
		// printf("after read and adding header:\n");
		udpfd = socket(AF_INET, SOCK_DGRAM, 0);
		bzero(&remoteaddr, sizeof(remoteaddr));
		remoteaddr.sin_family = AF_INET;
		remoteaddr.sin_port = htons(2152);
		
		inet_pton(AF_INET, argv[1], &remoteaddr.sin_addr);
		//printf("%d\n",setsockopt(udpfd,SOL_SOCKET,SO_BINDTODEVICE,"enp0s9",6));
		sendto(udpfd,sendbuffer,n+sizeof(gtp_header),0,(struct sockaddr*)&remoteaddr,sizeof(remoteaddr));
		free(sendbuffer);
		free(gtpuheader);
		free(recvbuffer);
		close(udpfd);
	}
}
