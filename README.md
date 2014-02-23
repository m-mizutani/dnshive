
DnsHive : Domain Name Oriented Traffic Monitor
============================================

Overview
--------------------------------------------
DnsHive monitors network traffic and gather results of DNS forward lookup, 
and provides network information according to results of DNS queries. 
Then you can see not only IP address of packet, but also original domain name of IP address.
It would help you to recognize what server your computer communicates with.


Example
--------------------------------------------
[example]:Output example of packet detail

### Reverse lookup based display (like tcpdump)

    IP nrt19s12-in-f8.1e100.net.https > 10.0.0.129.55440
	IP 10.0.0.129.55440 > nrt19s12-in-f8.1e100.net.https
	IP 10.0.0.129.55446 > nrt04s06-in-f21.1e100.net.http
	IP nrt04s06-in-f21.1e100.net.http > 10.0.0.129.55446

### Forward lookup based display (DnsHive)

    TCP encrypted-tbn3.gstatic.com.(74.125.235.168)/443 -> 10.0.0.129/55440
	TCP 10.0.0.129/55440 -> encrypted-tbn3.gstatic.com.(74.125.235.168)/443
	TCP 10.0.0.129/55446 -> gmail.com.(173.194.126.181)/80
	TCP gmail.com.(173.194.126.181)/80 -> 10.0.0.129/55446

Major Functions
--------------------------------------------

- Traffic monitoring and output packet detail based on DNS forward lookup (see [Example][example])
- Store DNS query and response to redis server, and Load old history of DNS records when restarting
- Output logs of DNS response records (A/AAAA/CNAME) and Flow start/end to ZeroMQ PUB or to file of msgpack format

Install
--------------------------------------------

### Requires
- hiredis (ver 0.11.0)
- msgpack (ver 5.7)
- zeromq (ver 2.x)
- libswarm (install from [github](https://github.com/m-mizutani/swarm))
- cmake (ver 2.8.x)

Please install redis-server, hiredis, msgpack and zeromq on your OS. An example of installation in Ubuntu is below.

    % sudo aptitude install redis-server libhiredis0.10 libhiredis-dev libmsgpack-dev libmsgpack3 build-essential libzmq1 libzmq-dev cmake libpcap-dev

### Setup

Install Swarm

    % git clone https://github.com/m-mizutani/swarm.git
    % cd swarm
    % cmake . && make
	% sudo make install


Install DnsHive

    % git clone https://github.com/m-mizutani/dnshive.git
    % cd dnshive
    % cmake . && make
    % sudo make install

Usage
--------------------------------------------

### Capture packets from network interface, and saving DNS 

    % sudo dnshive -i eth0 -d 0

The command line specifies to monitor `eth0`, and to insert query record into Redis server index `0`. When re-executing the command line after the process quits, the new process loads query data from Reids DB and re-construct name lookup database in the process.

### Read pcap file

    % dnshive -r somefile.pcap

The command specifies pcap format file `somefile.pcap` and output traffic.

### Output events of flow and DNS record to msgpack stream as file

    % dnshive -r somefile.pcap -q -m somefile.msg
	
### Output events of flow and DNS record to ZeroMQ publish socket.

    % dnshive -i eth0 -q -z "*:9000"

