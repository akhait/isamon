#include <string>
#include <stdlib.h>

using namespace std;

struct Config {
  string netAddr;
  unsigned long binNetAddr;
  int subnet;
  string interface = "";
  int port = 0;
  int wait = 0;
  bool mod_tcp = false;
  bool mod_udp = false;
};

const char *addrToStr(unsigned long);
unsigned int calcSubnetMask(int);
int checkArgs(int, char**, Config*);
int getNetClients(Config*);
