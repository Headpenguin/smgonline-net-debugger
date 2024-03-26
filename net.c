typedef unsigned long size_t;

#include "net.h"

#include <revolution/ipc/ipcclt.h>
#include "cstring"
#include "mem.h"

static const char *kd_fs = "/dev/net/kd/request";
static const char *iptop_fs = "/dev/net/ip/top";

static long iptop_fd = -1;
static long ipaddr = 0;

#define MAX_IP_ADDR_RETRY 3

#define IOCTL_NWC24iStartupSocket 0x06

#define IOCTL_SOConnect 0x04
#define IOCTL_SORecvFrom 0x0C
#define IOCTL_SOSendTo 0x0D
#define IOCTL_SOSocket 0x0F
#define IOCTL_SOGetHostID 0x10
#define IOCTL_SOStartup 0x1F
//#define IOCTL_SOShutown 0x0E

#define ENXIO 6
#define ETIMEDOUT 110
#define EINVAL 22

long netinit() {
    long kd_fd = -1, err = 0, i = 0;
    unsigned char kd_out[32] __attribute__((aligned(32))); //unused

    // NWC24 socket initialization

    kd_fd = IOS_Open(kd_fs, 0);

    if(kd_fd < 0) return kd_fd;

    err = IOS_Ioctl(kd_fd, IOCTL_NWC24iStartupSocket, NULL, 0, kd_out, 32);

    if(err < 0) {
        IOS_Close(kd_fd);
        kd_fd = -1;
        return err;
    }

    IOS_Close(kd_fd);
    kd_fd = -1;

    // SOStartup

    iptop_fd = IOS_Open(iptop_fs, 0);

    if(iptop_fd < 0) return iptop_fd;

    err = IOS_Ioctl(iptop_fd, IOCTL_SOStartup, NULL, 0, NULL, 0);

    if(err < 0) {
        IOS_Close(iptop_fd);
        iptop_fd = -1;
        return err;
    }

    // IP
    err = 0;

    for(i = 0; i < MAX_IP_ADDR_RETRY && err == 0; i++) {

        err = IOS_Ioctl(iptop_fd, IOCTL_SOGetHostID, NULL, 0, NULL, 0);

    }

    if(err == 0) {
        IOS_Close(iptop_fd);
        iptop_fd = -1;
        return -ETIMEDOUT;
    }

    ipaddr = err;
    
    // Done

    return 0;

}

/*long netshutdown(void) {
    long err = 0, ret = 0;

    if(iptop_fd >= 0) {
        err = IOS_Ioctl(iptop_fd, IOCTL_SOShutdown, NULL, 0, NULL, 0);
        ret = IOS_Close(iptop_fd);
        if(err < 0) ret = err;
    }

    return ret;
}*/

long netsocket(long domain, long type, long protocol) {
    long buff[3] __attribute__((aligned(32)));

    if(iptop_fd < 0) return -ENXIO;

    buff[0] = domain;
    buff[1] = type;
    buff[2] = protocol;

    return IOS_Ioctl(iptop_fd, IOCTL_SOSocket, buff, 12, NULL, 0);
};

struct connect_data {
    long fd;
    unsigned long has_addr;
    unsigned char addr[28];
};

long netconnect(long fd, struct sockaddr_in *addr) {
    struct connect_data buff __attribute((aligned(32)));

    if(iptop_fd < 0) return -ENXIO;
    
    memset(&buff, 0, sizeof(buff));

    buff.fd = fd;
    buff.has_addr = 1;

    memcpy(&buff.addr, addr, sizeof(*addr));
    
    return IOS_Ioctl(iptop_fd, IOCTL_SOConnect, &buff, sizeof(buff), NULL, 0);
}

struct sendto_data {
    long fd;
    unsigned long flags;
    unsigned long has_addr;
    unsigned char addr[28];
};

long netwrite(long fd, const void *data, size_t len) {
    struct sendto_data params __attribute((aligned(32)));
    IOSIoVector v[2] __attribute((aligned(32)));

    if(iptop_fd < 0) return -ENXIO;

    if((unsigned int)data % 32) return -EINVAL; // not aligned
                        
    memset(&params, 0, sizeof params);
    params.fd = fd;
    params.has_addr = 0;

    v[0].base = data;
    v[0].length = len;
    v[1].base = &params;
    v[1].length = sizeof params;

    return IOS_Ioctlv(iptop_fd, IOCTL_SOSendTo, 2, 0, v);
}

long netread(long fd, void *data, size_t len) {
    unsigned int params[2] __attribute((aligned(32))) = {fd, 0};
    IOSIoVector v[3] __attribute((aligned(32))) = {
        {&params, 8},
        {data, len},
        {NULL, 0} // Out - Address (recvfrom is not implemented)
    };

    if(iptop_fd < 0) return -ENXIO;

    if((unsigned int)data % 32) return -EINVAL; // not aligned
                                               
    return IOS_Ioctlv(iptop_fd, IOCTL_SORecvFrom, 1, 2, v);
}

