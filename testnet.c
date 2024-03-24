typedef unsigned long size_t;

#include "net.h"
#include "revolution/ipc/ipcclt.h"
#include "cstring"

static const char *msg = "plppppppp";

void testNet(void) {
    long fd;
    struct sockaddr_in server_addr = {
        8,
        1,
        5000,
        0xC0A8C00D
    };
    char buff[10] __attribute__((aligned(32)));

    memcpy(buff, msg, 10);

    netinit();
    fd = netsocket(2, 2, 0);
    netconnect(fd, &server_addr);
    IOS_Write(fd, buff, 10);
}

