#include "packets.h"
#include "net.h"
#include "cstring"

#define ENOMEM 12
#define ENXIO 6
#define ENOBUFS 105

static void *readbuff;
static long sockfd = -1;
#define READBUFF_SIZE 1500

static struct sockaddr_in server_addr = {
    8,
    2,
    5000,
    0x0A000060
};

void* __nw__FUli(unsigned int, int);

/*
    Transmission structure: PACKET_DISC, packet_*, [data1], ... 
*/

long packet_init(void) {
    long err = 0;
    char buff[8] __attribute((aligned(32))) = "connect";
    err = netinit();
    if(err < 0) return err;
    sockfd = netsocket(2, 2, 0);
    if(sockfd < 0) return sockfd;
    err = netconnect(sockfd, &server_addr);
    if(err < 0) return err;
    readbuff = __nw__FUli(READBUFF_SIZE, 32);
    if(!readbuff) return -ENOMEM;
//    memcpy(readbuff, "connect", 8);
    err = netwrite(sockfd, buff, 8);
    if(err < 0) return err;
    return 0;
}

long packet_mainloop(void) {
    long err = 0, len = 0;
    unsigned int rwaddr = 0, rwlen = 0;
    struct packet_transmission *transmission = readbuff;

    if(!readbuff || sockfd < 0) return -ENXIO;
    
    while(1) {
        err = netread(sockfd, readbuff, READBUFF_SIZE);
        
        if(err < 0) return err;

        len = err;

        if(len < sizeof transmission->disc) continue;

        if(len > READBUFF_SIZE) return -ENOBUFS;

        switch(transmission->disc) {
            case PACKET_DISC_STOP:
                // done reading
                transmission->disc = PACKET_DISC_ACK;
                transmission->ack.len = 0;
                return netwrite(sockfd, readbuff, sizeof transmission->disc + sizeof transmission->ack);
            case PACKET_DISC_ACK:
                // done reading
                break;
            case PACKET_DISC_READ:
                if(len < sizeof transmission->disc + sizeof transmission->read) continue;
                rwaddr = transmission->read.addr;
                rwlen = transmission->read.len;
                // done reading
                if(rwlen > sizeof transmission->disc + sizeof transmission->ack) continue;
                transmission->disc = PACKET_DISC_ACK;
                transmission->ack.len = rwlen;
                memcpy(readbuff + sizeof transmission->disc + sizeof transmission->ack, (const void *)rwaddr, rwlen);
                err = netwrite(sockfd, readbuff, sizeof transmission->disc + sizeof transmission->ack + rwlen);
                if(err < 0) return err;
                break;
            case PACKET_DISC_WRITE:
                if(len < sizeof transmission->disc + sizeof transmission->write) continue;
                rwaddr = transmission->write.addr;
                rwlen = transmission->write.len;
                if(len < sizeof transmission->disc + sizeof transmission->write + rwlen) continue;
                memcpy((void *)rwaddr, readbuff + sizeof transmission->disc + sizeof transmission->write, rwlen);
                // Fun cache invalidation instructions go here
                
                // done reading
                transmission->disc = PACKET_DISC_ACK;
                transmission->ack.len = 0;
                err = netwrite(sockfd, readbuff, sizeof transmission->disc + sizeof transmission->ack);
                if(err < 0) return err;
                break;
        }
    }
}

