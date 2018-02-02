# Usage:
```
 isamon [-h] [-i <interface>] [-t] [-u] [-p <port>] [-w <ms>] -n <net_address/mask>
    -h --help -- show this help
    -i --interface <interface> -- scan using specific interface
    -n --network <net_address/mask> -- IP address and submask that define scanning range
    -t --tcp -- use TCP scan
    -u --udp -- use UDP scan
    -p --port <port> -- port that will be scanned. If not provided, all ports from range 1−65535 will be scanned
    -w --wait <ms> -- max RTT
```
# Examples:
```$ isamon -n 192.168.1.0/30 -t -u -w 5```
192.168.1.1 
192.168.1.1 TCP 80
192.168.1.1 TCP 22 
192.168.1.1 UDP 53 

```$ isamon -n 192.168.1.0/30 ```
192.168.1.1 
192.168.1.2 
Stderr

Err code
- 0 - OK
- -1 - error, check errno for more details
