#ifndef NET_H
#define NET_H

typedef long size_t;

struct sockaddr_in {
    unsigned char addrlen;
    unsigned char addrfamily;
    short port;
    long addr;
};

long netinit(void);
long netsocket(long domain, long type, long protocol);
long netconnect(long fd, struct sockaddr_in *addr);
// Please align `data` to 32 bytes
long netwrite(long fd, const void *data, size_t len);
// Please align `data` to 32 bytes
long netread(long fd, void *data, size_t len);

//long netshutdown(void);
#endif
