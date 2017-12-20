/* Copyright (c) 2017 Nathaniel Clark
 *
 * network 
 *
 */

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "xt.h"

#define NET_SIZE(_a) (((_a)->type == AF_INET) ?sizeof((_a)->addr.in) :sizeof((_a)->addr.in6))

int str2addr(struct address *addr, const char *buf)
{
    int n, p;
    char ip[INET_ADDRSTRLEN], mask[INET_ADDRSTRLEN];

    if (strchr(buf, '/')) {
	n = sscanf(buf, "%[0-9a-fA-F.:]/%d", ip, &p);
	if (n < 1)
	    return -EINVAL;
    } else if (strchr(buf, ':')) { // IPv6
	sscanf(buf, "%s", ip);
	p = INT_MAX;
    } else { // IPv4
	/* Need to account for old partial network definitions. e.g.
	 * 192.168 LAT LON # COMMENT
	 */
	int a=0, b=0, c=0, d=0;
	n = sscanf(buf, "%d.%d.%d.%d.%d", &a, &b, &c, &d);
	if (n < 1)
	    return -EINVAL;
	sprintf(ip, "%d.%d.%d.%d", a, b, c, d);
	p = (d>0) ?32 :(c>0) ?24 :(b>0) ?16 :8;
    }

    if (inet_pton(AF_INET, ip, &(addr->addr.in)))
	addr->type = AF_INET;

    else if (inet_pton(AF_INET6, ip, &(addr->addr.in6)))
	addr->type = AF_INET6;

    else {
	printf("ERROR:s2a: NO parse IP:%s (prefix:%d)\n", ip, p);
	return -EINVAL;
    }

    addr->prefix = MIN(p, NET_PREFIX(*addr));

    DPRINTF("s2a: %s is type:%d prefix:%d\n", buf, addr->type, addr->prefix);

    return 0;
}

int addr2str(struct address *addr, char *buf, int size)
{
    char tmp[IP_STR_LEN];

    if (addr->type == 0) {
	buf[0] = '\0';
	return 0;
    }

    if ((addr->type == AF_INET && addr->prefix < 32) ||
	(addr->type == AF_INET6 && addr->prefix < 128))
	return snprintf(buf, size, "%s/%d",
			inet_ntop(addr->type, &addr->addr, tmp, sizeof(tmp)),
			addr->prefix);
    else
	return snprintf(buf, size, "%s",
			inet_ntop(addr->type, &addr->addr, tmp, sizeof(tmp)));
}

/* from ipcalc.c */
static uint32_t prefix2mask(int prefix)
{
	struct in_addr mask;
	memset(&mask, 0, sizeof(mask));
	if (prefix)
		return htonl(~((1 << (32 - prefix)) - 1));
	else
		return htonl(0);
}


int addrInAddr(struct address *net, struct address *ip)
{
    int mask, i, j;
    uint8_t c;
    
    if (net->type != ip->type) {
	return FALSE;
    }

    switch (net->type) {
    case AF_INET:
	mask = prefix2mask(net->prefix);
	DPRINTF("Checking %s NET:%08x/%d (mask:%08x) IP:%08x/%d\n",
		inet_ntoa(net->addr.in),
		net->addr.in.s_addr, net->prefix, mask,
		ip->addr.in.s_addr, ip->prefix);
	return ((net->addr.in.s_addr & mask) == (ip->addr.in.s_addr & mask));
	
    case AF_INET6:
	for (i = net->prefix, j = 0; i > 0; i -= 8, j++) {
	    if (i >= 8)
		c = 0xff;
	    else
		c = 0xffU << (8 - i);
	    if (net->addr.in6.s6_addr[j] & c == ip->addr.in6.s6_addr[j] & c)
		return FALSE;
	}
	return TRUE;
    }
    
    return FALSE;
}
