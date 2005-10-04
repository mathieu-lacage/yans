/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef POSIX_SOCKET_H
#define POSIX_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif 

/* This is the only socket function which was changed. to take a
 * struct Host * argument. This argument is in fact a class Host
 * as defined in host.h
 */
int posix_socket(struct Host *host, int domain, int type, int protocol);
int posix_bind(int sockfd, struct sockaddr *my_addr, 
	       socklen_t addrlen);
int posix_getsockname (int fd, struct sockaddr *addr, 
		       socklen_t *len);
int posix_connect (int fd, struct sockaddr const * addr, 
		   socklen_t len);
int posix_getpeername (int fd, struct sockaddr * addr,
		       socklen_t *len);
ssize_t posix_send (int fd, const void *buf, size_t n, int flags);
ssize_t posix_recv (int fd, void *buf, size_t n, int flags);
ssize_t posix_sendto (int fd, const void *buf, size_t n,
		      int flags, struct sockaddr const * addr,
		      socklen_t addr_len);
ssize_t posix_recvfrom (int fd, void *buf, size_t n,
			int flags, struct sockaddr * addr,
			socklen_t *addr_len);
ssize_t posix_sendmsg (int fd, const struct msghdr *message, 
		       int flags);
ssize_t posix_recvmsg (int fd, struct msghdr *message, 
		       int flags);
int posix_getsockopt (int fd, int level, int optname,
		      void *optval,
		      socklen_t *optlen);
int posix_setsockopt (int fd, int level, int optname,
		      const void *optval, socklen_t optlen);
int posix_listen (int fd, int n);
int posix_accept (int fd, struct sockaddr *addr, 
		  socklen_t *addr_len);
int posix_shutdown (int fd, int how);
int posix_sockatmark (int fd);
int posix_isfdtype (int fd, int fdtype);

#ifdef __cplusplus
}
#endif 

#endif /* POSIX_SOCKET_H */
