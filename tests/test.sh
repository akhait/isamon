#!/bin/bash
# Tests for isamon
# 192.168.1.0 - loopback
# 192.168.1.1
# 192.168.1.2
# 192.168.1.3 - broadcast

# Test plan:
# 1. isamon -h
# 2. isamon -n 192.168.1.0/30
# 3. isamon -n 192.168.1.0/30 -t
# 3. isamon -n 192.168.1.0/30 -u
# 4. isamon -n 192.168.1.0/30 -u -t
# 5. isamon -n 192.168.1.0/30 -t -p 5000
# 6. isamon -n 192.168.1.0/30 -u -p 5000
# 7. isamon -n 192.168.1.0/30 -t -u -p 5000
#TODO: test -i
#TODO: test -w


