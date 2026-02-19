#ifndef _NET_IN_H_
#define _NET_IN_H_

#include <types.h>

#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 2
#define BYTE_ORDER LITTLE_ENDIAN

/* Internet address */
struct in_addr {
    uint32_t s_addr; /* address in network byte order */
};

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);

uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

#endif
