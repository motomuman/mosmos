#ifndef _NETDEV_H_
#define _NETDEV_H_

#include "ether.h"
#include "pktbuf.h"

struct net_device {
	uint32_t ip_addr;
	int netmask;
	uint32_t gw_addr;
	uint8_t hw_addr[ETHER_ADDR_LEN];

	void (*tx) (struct pktbuf *pkt);
};

void netdev_set_ip_addr(uint32_t ip_addr);
void netdev_set_gw_addr(uint32_t gw_addr);
void netdev_set_netmask(int netmask);
void netdev_set_tx_handler(void (tx) (struct pktbuf *pkt));
void netdev_set_hw_addr(uint8_t hw_addr[]);
struct net_device *get_netdev();

#endif
