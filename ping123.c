#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include<sys/time.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define PACKETSIZE	64
clock_t star1t, en1d;
struct timeval recv_tout;
struct packet
{
	struct icmphdr hdr;
	char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;
struct protoent *proto=NULL;
double cpu_time_used;
unsigned short checksum(void *b, int len)
{	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}


void ping(struct sockaddr_in *addr)
{	const int val=255;
	int i, sd, cnt=1;
	struct packet pckt,pckt2;
	struct sockaddr_in r_addr,addr12;
	unsigned char buf[1024];
	recv_tout.tv_sec=2;
	recv_tout.tv_usec=0;
	sd = socket(2, SOCK_RAW, proto->p_proto);
	if ( sd < 0 )
	{
		perror("socket");
		return;
	}
    
	if ( setsockopt(sd, 0, 2, &val, sizeof(val)) != 0)
		perror("Set TTL option");
	if ( setsockopt(sd, 1,20, &recv_tout, sizeof(recv_tout)) != 0)
		perror("Set TTL option");
	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");
	for (int q=0;q<10;q++)
	{	
		int len=sizeof(r_addr),length;
		bzero(&pckt, sizeof(pckt));
		pckt.hdr.type = 8;
		pckt.hdr.un.echo.id = 9999;
		for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
			pckt.msg[i] = i+'0';

		pckt.msg[i] = 0;
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
	   	star1t = clock();
		if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");

		printf("Message #%d\n", cnt-1);
		while(1) {
			int bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&r_addr, &len);
			struct iphdr *ip = buf;
			struct icmphdr *icmp =  buf+ ip->ihl*4;
			while(icmp->un.echo.sequence<cnt-1)
			{
				int bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&r_addr, &len);
				struct iphdr *ip = buf;
				struct icmphdr *icmp =  buf+ ip->ihl*4;
				if(icmp->code == 0 && icmp->type== 0 && icmp->un.echo.id==9999) {
				if(icmp->un.echo.sequence==(cnt-1))
				break;
				}	
			}
			if(icmp->code == 0 && icmp->type== 0 && icmp->un.echo.id==9999) {
				printf("Got reply\n");
				printf("seq : %d\n",icmp->un.echo.sequence);}
				en1d = clock();
				cpu_time_used = (((double) (en1d - star1t)) / CLOCKS_PER_SEC)*1000;
		  		printf("ping time : %f ms ",cpu_time_used);
			break;
		}
		sleep(1);         
	}
	
}

int main(int count, char *strings[])
{	struct hostent *hname;
	struct sockaddr_in addr;

	if ( count != 2 )
	{
		printf("usage: %s <addr>\n", strings[0]);
		exit(0);
	}
	if ( count > 1 )
	{
		pid = getpid();
		proto = getprotobyname("ICMP");
		hname = gethostbyname(strings[1]);
		bzero(&addr, sizeof(addr));
		addr.sin_family = hname->h_addrtype;
		addr.sin_port = 10;
		addr.sin_addr.s_addr = *(long*)hname->h_addr_list[0];
	    ping(&addr);
	}
	else
		printf("usage: myping <hostname>\n");
	return 0;
}