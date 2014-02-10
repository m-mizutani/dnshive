DnsHive : Domain Name Oriented Traffic Monitor
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

Sample command line

    % dnshive -i eth0 -d 0

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


LISENCE
--------------------------------------------

BSD 2-Clause license

Copyright (c) 2013 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

