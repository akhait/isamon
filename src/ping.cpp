#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>
#include "utils.h"
#include "ping.h"
#include "main.h"

int ping(unsigned int ip, const char *iface, int wait) {	
	const int val=255; //ttl value
	int sockfd, recvfd, ret=1, i, cnt=1, pid = getpid();
	struct packet pckt;
	struct sockaddr_in addr, r_addr;
	//Set up socket
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
		error("socket");
	//Set socket options
	if (setsockopt(sockfd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
		error("Set TTL option");
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) != 0 )
		error("Request nonblocking I/O");
    if ( setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) < 0)
        error("setsockopt(SO_BINDTODEVICE)");
	//Set up sockaddr struct
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ip);
	socklen_t len=sizeof(r_addr);
	//Send ping
	for (int j = 0; j < 5; j++) {
		bzero(&pckt, sizeof(pckt));
    	pckt.hdr.type = ICMP_ECHO;
    	pckt.hdr.un.echo.id = pid;
    	for (i = 0; i < sizeof(pckt.msg)-1; i++ )
        	pckt.msg[i] = i+'0';
    	pckt.msg[i] = 0;
    	pckt.hdr.un.echo.sequence = cnt++;
    	pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
    	if ( sendto(sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr, sizeof(addr)) <= 0 )
        	error("sendto");
	}
	//Recieve reponse
	unsigned char *recvpckt = (unsigned char *)malloc(IP_MAXPACKET * sizeof (unsigned char));
	if((ret = recvICMP(sockfd, recvpckt, 20+8+56, wait)) > 0) {
		//Decode recieved packet
		struct ip *ipp = (struct ip *)recvpckt;
  		struct icmphdr *icmp = (struct icmphdr *)(recvpckt + 20);
		if (ipp->ip_p == 0x01 && icmp->type == 0)
			printf("%s\n", addrToStr(ip));
	}
	else if (ret < 0)
        error("recvfrom");
	//Close socket
	close(sockfd);
	return ret;
}

int pingTCP(unsigned int ip, int port, const char *iface, int wait) {
    int sockfd, recvfd, ret;
    struct sockaddr_in saddr;
    char datagram[4096], src_ip[20];
	//Set up socketaddr_in struct
    bzero((char *) &saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(ip);
    saddr.sin_port = htons(port);
	//Create socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0) 
        error("socket(SOCK_RAW, IPPROTO_TCP) - sockfd");
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) != 0 )
        error("Request nonblocking I/O");
	//Set socket options
	if ( setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) < 0)
        error("setsockopt(SO_BINDTODEVICE)");
    const int val = 1;
    if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, &val, sizeof (val)) < 0)
        error("setsockopt(IP_HDRINCL)");
	//Set up IP header
	struct iphdr *iph = (struct iphdr *)datagram;
    ipHdr(iph);
	getLocalIp(src_ip);
	iph->saddr = inet_addr(src_ip);    //Spoof the source ip address
    iph->daddr = saddr.sin_addr.s_addr;
	//Set up TCP header
	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
    tcpHdr(tcph);
	tcph->dest = htons (port);
	//Checksum calculations
    struct pseudo_header psh;
	psh.source_address = inet_addr(src_ip);
    psh.dest_address = saddr.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons( sizeof(struct tcphdr) );     
    memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));
    tcph->check = checksum( (unsigned short*) &psh , sizeof (struct pseudo_header));
	signal(SIGPIPE, SIG_IGN); //broken pipe error workaround
    //Send the packet
    if (sendto (sockfd, datagram, sizeof(struct iphdr) + sizeof(struct tcphdr), 0 ,(struct sockaddr *)&saddr, sizeof (saddr)) < 0)
		error("sendto()");
    //Receive response
    char buffer[44];
    if ((ret = recvICMP(sockfd, buffer, 44, wait)) > 0) {
		//Decode response
		struct iphdr *iphr = (struct iphdr*)buffer;
		int iphdrlen = iphr->ihl*4;
		struct tcphdr *tcphr=(struct tcphdr*)(buffer + iphdrlen);
		if (tcphr->syn && tcphr->ack)
			printf("%s TCP %d\n", addrToStr(ip), port);
	}
	else if (ret < 0)
        error("recvfrom");
	//Close sockets
    close(sockfd);
    return 0;
}

int pingUDP(unsigned int ip, int port, const char *iface, int wait) {
    int sockfd, recvfd, ret;
    struct sockaddr_in saddr, from;
    char buffer[256];
	bzero(&buffer, sizeof(buffer));
    //Create send UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
        error("socket(IPPROTO_UDP)");
	//Set socket options
	if ( setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) < 0)
		error("setsockopt(SO_BINDTODEVICE)");
	//Create reciece ICMP socket
	if((recvfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		error("socket(IPPROTO_ICMP)");
    //Set up socketaddr_in struct
    bzero((char *) &saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(ip);
    saddr.sin_port = htons(port);
	socklen_t len = sizeof(saddr);
	//Send ping
	if(sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
		error("sendto()");
	//Recieve ICMP
	if ((ret = recvICMP(recvfd, buffer, 84, wait)) > 0) {
		//Decode response
		struct ip *iphdr = (struct ip *)buffer;
    	int iplen = iphdr->ip_hl << 2;
		struct icmp *icmp = (struct icmp *)(buffer + iplen);
		if ((icmp->icmp_type != ICMP_UNREACH) && (icmp->icmp_code != ICMP_UNREACH_PORT))
			printf("%s %d UDP\n", addrToStr(ip), port);
	}
	else if (ret < 0)
		error("recvfrom");
	// Close socket
	close(sockfd);
	close(recvfd);
}

//Recieve ICMP messages
int recvICMP(int recvfd, void *buffer, int size, int wait) {
	int ret;
    fd_set readset;
    struct timeval time;
	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);
    time.tv_sec = 0;
    time.tv_usec = wait * 1000;
    FD_ZERO(&readset);
    FD_SET(recvfd, &readset);
    if ((ret = select(recvfd+1, &readset, NULL, NULL, &time)) < 0)
        error("select()");
    else if(ret == 0)
        return 0; //shut down in case of timeout
    else if(FD_ISSET(recvfd, &readset)) {
        ret = recvfrom(recvfd, (char *)buffer, size, 0, (struct sockaddr *)&addr, &len);
    	return ret;
	}
}
