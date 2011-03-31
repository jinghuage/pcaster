#ifndef _VSOCKET_H_
#define	_VSOCKET_H_

void set_nonblock(int socket_id);
void set_reuseaddr(int socket_id);
void set_recv_timeout(int socket_id, int ns, int nus);
void set_sendbufsize(int socket_id, int size);
void set_recvbufsize(int socket_id, int size);

#endif
