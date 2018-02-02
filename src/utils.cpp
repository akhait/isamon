#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include "utils.h"

/*------------------------------------
 * HELPER FUNCTIONS 
-------------------------------------*/
//Help
void help() {
	printf("Usage:\n");
	printf("isamon [-h] [-i <interface>] [-t] [-u] [-p <port>] [-w <ms>] -n <net_address/mask>\n"); 
    printf("\t-h --help -- show this help\n"); 
   	printf("\t-i --interface <interface> -- scan using specific interface\n");
    printf("\t-n --network <net_address/mask> -- IP address and submask that define scanning range\n"); 
    printf("\t-t --tcp -- use TCP scan\n");
   	printf("\t-u --udp -- use UDP scan\n");
    printf("\t-p --port <port> -- port that will be scanned. If not provided, all ports from range 1−65535 will be scanned\n"); 
    printf("\t-w --wait <ms> -- max RTT\n");
}

//Error handler
void error(const char *msg) {
    perror(msg);
    exit(-1);
}

//convert network address to string representation
const char* addrToStr(unsigned long addr) {
    struct in_addr paddr;
    paddr.s_addr = htonl(addr);
    return inet_ntoa(paddr);
}

//convert int subnet to binary representation (ex. 24 to b1111111111111111111111100000000)
unsigned int calcSubnetMask(int subn) {
    unsigned int subnet = 0;
    for(int i = 0; i < subn; i++)
        subnet += pow(2, 31-i);
    return subnet;
}

//Get ip of localhost (used for constructing IP/TCP headers)
void getLocalIp ( char * buffer) {
    int sock = socket ( AF_INET, SOCK_DGRAM, 0);
    const char* kGoogleDnsIp = "8.8.8.8";
    int dns_port = 53;
    struct sockaddr_in serv;
    //Set up serv struct
    memset( &serv, 0, sizeof(serv) );
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons( dns_port );
    //Connect
    int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
    //Get ip
    const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
    //Close socket
    close(sock);
}

//Simple one's complemet checksum calculation
unsigned short checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
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

//Sets all required fields in IP header to appropriate values
void ipHdr(struct iphdr *iph) {
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
    iph->id = htons (54321); //Id of this packet
    iph->frag_off = htons(16384);
    iph->ttl = 225;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;      //Set to 0 before calculating checksum
}

//Sets all required fields in TCP header to appropriate values
void tcpHdr(struct tcphdr *tcph) {
    tcph->source = htons(43591);
    tcph->seq = htonl(1105024978);
    tcph->ack_seq = 0;
    tcph->doff = sizeof(struct tcphdr) / 4;      //Size of tcp header
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (14600);  // maximum allowed window size
    tcph->check = 0;
    tcph->urg_ptr = 0;
}
