#include "netdev.h"
#include "lib.h"

//Currently, support only single netdevice..

struct net_device netdev;

void netdev_set_ip_addr(uint32_t ip_addr)
{
	netdev.ip_addr = ip_addr;
}

void netdev_set_gw_addr(uint32_t gw_addr)
{
	netdev.gw_addr = gw_addr;
}

void netdev_set_netmask(int netmask)
{
	netdev.netmask = netmask;
}

void netdev_set_tx_handler(void (tx) (struct pktbuf *pkt))
{
	netdev.tx = tx;
}

void netdev_set_hw_addr(uint8_t *mac_addr)
{
	memcpy(netdev.hw_addr, mac_addr, ETHER_ADDR_LEN);
}

struct net_device *get_netdev() {
	return &netdev;
}
