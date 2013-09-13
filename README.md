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

    TCP 10.0.0.2:51694 > iad23s07-in-f1.1e100.net.https:
    TCP iad23s07-in-f1.1e100.net.https > 10.0.0.2:51694
    TCP 10.0.0.2:51694 > iad23s07-in-f1.1e100.net.https:

### Forward lookup based display (DnsHive)

    TCP my-macosx.local:51694 > www.google.com.https:
    TCP www.google.com.https > my-macosx.local:51694
    TCP my-macosx.local:51694 > www.google.com.https:
    

Usage
--------------------------------------------

Under development
