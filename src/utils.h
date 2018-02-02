void help();
void error(const char *);
const char* addrToStr(unsigned long);
unsigned int calcSubnetMask(int);
void getLocalIp (char *);
unsigned short checksum(void *, int);
void ipHdr(struct iphdr *) ;
void tcpHdr(struct tcphdr *);
