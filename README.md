DnsHive
============================================

Overview
--------------------------------------------
DnsHive monitors network traffic and gather results of DNS forward lookup, 
and provides network information according to DNS query resutls. 
Then you can recognize what server dose your computer communicate.


Example
--------------------------------------------

### Reverse lookup based display (like tcpdump)

    TCP 10.0.0.2/51694 > iad23s07-in-f1.1e100.net/443
    TCP iad23s07-in-f1.1e100.net/443 > 10.0.0.2/51694
    TCP 10.0.0.2/51694 > iad23s07-in-f1.1e100.net/443

### Forward lookup based display (DnsHive)

    TCP my-macosx.local/51694 > www.google.com/443
    TCP www.google.com/443 > my-macosx.local/51694
    TCP my-macosx.local/51694 > www.google.com/443

Install
--------------------------------------------

### Requires
- libpcap
- redis-server
- hiredis
- msgpack 
- libswarm (install from [github](https://github.com/m-mizutani/swarm))

Please install libpcap, redis-server, hiredis and msgpack based on your OS's architecture. An example of installation in Ubuntu is below.

    % sudo aptitude install redis-server libhiredis0.10 libhiredis-dev libmsgpack-dev libmsgpack3 build-essential cmake

### Setup

    % git clone https://github.com/m-mizutani/swarm.git
    % cd swarm
    % cmake . && make
    % cd ..
    % git clone https://github.com/m-mizutani/dnshive.git
    % cd dnshive
    % cmake -DINC_DIR=../swarm/src -DLIB_DIR=../swarm/lib . 
    % make
    % sudo make install

Usage
--------------------------------------------

Sample command line

    % dhive -i eth0 -d 0

The command line specify to monitor `eth0`, and to insert query record into Redis server index `0`. When re-executing the command line after the process quits, the new process loads query data from Reids DB and re-construct name lookup database in the process.

    % dhive -h
    Usage: dhive [options]
    
    Options:
      -h, --help            show this help message and exit
      -r STRING             Specify read pcap format file(s)
      -i STRING             Specify interface to monitor on the fly
      -d INT                Redis DB Index (MUST be set if you want redis DB
      -h STRING             Reids DB Host, default is localhost
      -p INT                Reids DB Port, default is 6379
