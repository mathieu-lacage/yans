/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SIMU_SOCKET_H
#define SIMU_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif 

/* This is the only socket function which was changed. to take a
 * struct Host * argument. This argument is in fact a class Host
 * as defined in host.h
 */
int simu_socket(struct Host *host, int domain, int type, int protocol);
int simu_bind(int sockfd, struct sockaddr *my_addr, 
	      socklen_t addrlen);
int simu_getsockname (int fd, struct sockaddr *addr, 
		      socklen_t *len);
int simu_connect (int fd, struct sockaddr const * addr, 
		  socklen_t len);
int simu_getpeername (int fd, struct sockaddr * addr,
		      socklen_t *len);
ssize_t simu_send (int fd, const void *buf, size_t n, int flags);
ssize_t simu_recv (int fd, void *buf, size_t n, int flags);
ssize_t simu_sendto (int fd, const void *buf, size_t n,
		     int flags, struct sockaddr const * addr,
		     socklen_t addr_len);
ssize_t simu_recvfrom (int fd, void *buf, size_t n,
		       int flags, struct sockaddr * addr,
		       socklen_t *addr_len);
ssize_t simu_sendmsg (int fd, const struct msghdr *message, 
		      int flags);
ssize_t simu_recvmsg (int fd, struct msghdr *message, 
		      int flags);
int simu_getsockopt (int fd, int level, int optname,
		     void *optval,
		     socklen_t *optlen);
int simu_setsockopt (int fd, int level, int optname,
		     const void *optval, socklen_t optlen);
int simu_listen (int fd, int n);
int simu_accept (int fd, struct sockaddr *addr, 
		 socklen_t *addr_len);
int simu_shutdown (int fd, int how);
int simu_sockatmark (int fd);
int simu_isfdtype (int fd, int fdtype);

#ifdef __cplusplus
}
#endif 

#endif /* SIMU_SOCKET_H */
