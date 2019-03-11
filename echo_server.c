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
    if (argc != 2)
	{
		printf("usage: echo_server <bind port>\n");
		return -1;
	}

	int sockfd,n;
	
	struct sockaddr_in servaddr,remoteaddr;
	char recvbuffer[1600],sendbuffer[2000];
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));

	bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_port = htons(2152);
	

    while(1){
		n = recvfrom(sockfd, recvbuffer,1600, MSG_WAITALL, NULL, NULL);
		if(n == -1){
			printf("Received data %d\n",n);
			
			continue;
			// perror(errno);
		}
		struct ip* ip_header = (struct ip*) recvbuffer;

		printf("Received data %d\n",n);
		remoteaddr.sin_addr = ip_header->ip_src;

		sendto(sockfd,sendbuffer,n,0,(struct sockaddr*)&remoteaddr,sizeof(remoteaddr));

	}
	
}