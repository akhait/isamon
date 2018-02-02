#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header

//Represents IP packet with ICMP message
#define PACKETSIZE  64
struct packet
{
    struct icmphdr hdr;
    char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

//Needed for checksum calculation
struct pseudo_header
{
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;
     
    struct tcphdr tcp;
};

int ping(unsigned int, const char *, int);
int recvICMP(int, void*, int, int);
int pingTCP(unsigned int, int, const char *, int);
int pingUDP(unsigned int, int, const char *, int);
