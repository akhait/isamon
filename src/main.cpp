#include <iostream>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h>
#include "utils.h"
#include "main.h"
#include "ping.h"
#define MIN_PORT 1
#define MAX_PORT 65535
using namespace std;
const struct option longopts[] =
{
    {"network", 1, 0, 'n'},
    {"port", 1, 0, 'p'},
    {"interface", 1, 0, 'i'},
    {"wait", 1, 0, 'w'},
	{"tcp", 0, 0, 't'},
	{"udp", 0, 0, 'u'},
	{"help", 0, 0, 'h'}
};

int main(int argc, char **argv) {
	Config conf;
	int ret;
	if((ret = checkArgs(argc, argv, &conf)) == -1)
		return ret;
	getNetClients(&conf);
	return 0;
}

int checkArgs(int argc, char **argv, Config *conf) {
	int c=0, index=0;
	while ((c = getopt_long(argc, argv, "n:p:i:w:tuh", longopts, &index)) != -1)
		switch (c) {
			case 'n': {
				//Network data:
				string network = optarg;
				//Network addres/subnet mask string	
				int pos = network.find("/");
				//Network address string
				conf->netAddr = network.substr(0,pos);
				//Subnet mask (integer)
				conf->subnet = atoi(network.substr(pos+1).c_str());
				//Constructing network struct
				sockaddr sa;
				if ((inet_pton(AF_INET, conf->netAddr.c_str(), 
					&(((sockaddr_in *)&sa)->sin_addr)) == 1))
					conf->binNetAddr = ((sockaddr_in *)&sa)->sin_addr.s_addr;
				else
				{
					fprintf(stderr, "Bad format of IP address - only IPv4 addresses allowed.\n");
					return -1;
				}
				break;
			}
			case 'p':
				conf->port = atoi(optarg);
				break;
			case 'i':
				conf->interface = optarg;
				break;
			case 't':
				conf->mod_tcp = true;
				break;
			case 'u':
				conf->mod_udp = true;
				break;
			case 'w':
				conf->wait = atoi(optarg);
				break;
			case 'h':
				help();
				exit(0);
		}
	if (conf->port && !(conf->mod_tcp || conf->mod_udp)) {
		fprintf(stderr, "Can't use -p/--port option without options -t/--tcp and/or -u/--udp\n");
        return -1;
    }
	return 0;
}

// Get network hosts addresses using subnet mask,
// and immidiately test if the host is active,
// execute further scans if needed.
int getNetClients(Config *conf) {
	//Get address network representation
	unsigned long ip = htonl(conf->binNetAddr);
	unsigned int subnet = calcSubnetMask(conf->subnet);
	//Network address
	unsigned long net_addr = ip & subnet;
	//Broadcast address
	unsigned long broad_addr = ip | ~subnet;
	
	for (unsigned long addr = net_addr+1; addr < broad_addr; addr++) {
		// if client is active, print out client's address
        ping(addr, conf->interface.c_str(), conf->wait);
		// scan TCP port(s)
		if (conf->mod_tcp) {
			if (conf->port)
				pingTCP(addr, conf->port, conf->interface.c_str(), conf->wait);
			else
				for (int port = MIN_PORT; port < MAX_PORT; port++) {
					pingTCP(addr, port, conf->interface.c_str(), conf->wait);
				}
		}
		// scan UDP port(s)
		if (conf->mod_udp) {
			if (conf->port)
                pingUDP(addr, conf->port, conf->interface.c_str(), conf->wait);
            else
                for (int port = MIN_PORT; port < MAX_PORT; port++)
                    pingUDP(addr, port, conf->interface.c_str(), conf->wait);
		}
	}
}
