#ifndef _RAW_H_
#define _RAW_H_

void raw_socket_init();
int raw_socket(uint8_t proto);
void raw_socket_free(int socket_id);
void raw_socket_send(int socket_id, uint32_t dip, uint8_t *buf, uint32_t size, uint8_t ttl);
int raw_socket_recv(int socket_id, uint8_t *buf, uint32_t size);
void raw_recv(struct pktbuf *pkt, uint8_t proto);

#endif
