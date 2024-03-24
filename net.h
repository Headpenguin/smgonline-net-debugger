#ifndef NET_H
#define NET_H

struct sockaddr_in {
    unsigned char addrlen;
    unsigned char addrfamily;
    short port;
    long addr;
};

long netinit(void);
long netsocket(long domain, long type, long protocol);
long netconnect(long fd, struct sockaddr_in *addr);

//long netshutdown(void);
#endif
