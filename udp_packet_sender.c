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
int main(int argc, char **argv)
{
	int sendfd,recvfd;
	struct sockaddr_in servaddr,local,cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	char sendline[1500],recvline[1500];
	if (argc != 3)
	{
		printf("usage: udp_packet_sender <gtp_server ip address> <gtp_server port>\n");
		return -1;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	// servaddr.sin_port = htons(5202);
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	local.sin_family = AF_INET;
	local.sin_port = htons(2152);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	sendfd = socket(AF_INET, SOCK_DGRAM, 0);

	for(int i=0;i<1500;++i){
		sendline[i] = '1';
	}

	while (1) {
		nanosleep(10000);
		// *sendline = "111111111111111111111111111111111";
		sendto(sendfd, sendline, strlen(sendline), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		// for(int i=0;i<10000;++i);
		// printf("packet send\n");
		// recvfd = socket(AF_INET, SOCK_DGRAM, 0);
		// bind(recvfd,(struct sockaddr*)&local,sizeof(local));	
		// recvfrom(recvfd,recvline,1500,0,(struct sockaddr*)&cliaddr, &cliaddrlen);
	}
	exit(0);
}
