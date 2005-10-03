/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <errno.h>
#include <netinet/in.h>
#include <cassert>

#include "sgi-hashmap.h"
#include "simu-socket.h"
#include "host.h"
#include "socket.h"
#include "utils.h"

static Sgi::hash_map<int, Socket *> g_socket_mapping;
static int g_allocated_socket_fds = 3;

int
socket_add (Socket *socket)
{
	int fd = g_allocated_socket_fds;
	g_socket_mapping[fd] = socket;
	g_allocated_socket_fds ++;
	return fd;
}

Socket *
socket_lookup (int fd)
{
	if (g_socket_mapping.find (fd) == g_socket_mapping.end ()) {
		return 0;
	} else {
		return g_socket_mapping[fd];
	}
}

int 
simu_socket(struct Host *host, int domain, int type, int protocol)
{
	assert (domain == AF_INET ||
		domain == AF_INET6);
	assert (type == SOCK_STREAM ||
		type == SOCK_DGRAM  ||
		type == SOCK_RAW);
	assert (protocol == IPPROTO_UDP  ||
		protocol == IPPROTO_TCP  ||
		protocol == IPPROTO_ICMP ||
		protocol == IPPROTO_RAW);
	Socket *socket = host->create_udp_socket ();
	return socket_add (socket);
}
int 
simu_bind(int sockfd, struct sockaddr *my_addr, 
	  socklen_t addrlen)
{
	Socket *socket = socket_lookup (sockfd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
	if (socket->is_ipv6 ()) {
		struct sockaddr_in6 *in6_addr = (struct sockaddr_in6 *)my_addr;
		if (addrlen != sizeof (struct sockaddr_in6) ||
		    in6_addr->sin6_family != AF_INET6) {
			errno = EINVAL;
			goto error;
		}
		// XXX
		assert (false);
		return -1;
	} else {
		struct sockaddr_in *in_addr = (struct sockaddr_in *)my_addr;
		if (addrlen != sizeof (struct sockaddr_in) ||
		    in_addr->sin_family != AF_INET) {
			errno = EINVAL;
			goto error;
		}
		Ipv4Address addr = utils_nstoh (in_addr->sin_addr.s_addr);
		uint16_t port = utils_nstoh (in_addr->sin_port);
		socket->bind (addr, port);
	}
	return 0;
 error:
	return -1;
}
int 
simu_getsockname (int fd, struct sockaddr *addr, 
		  socklen_t *addrlen)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
	if (socket->is_ipv6 ()) {
		if (*addrlen < sizeof (struct sockaddr_in6)) {
			errno = EINVAL;
			goto error;
		}
		struct sockaddr_in6 *in6_addr = (struct sockaddr_in6 *)addr;
		in6_addr->sin6_family = AF_INET6;
		*addrlen = sizeof (struct sockaddr_in6);
		// XXX
		assert (false);
		return -1;
	} else {
		if (*addrlen < sizeof (struct sockaddr_in)) {
			errno = EINVAL;
			goto error;
		}
		struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
		in_addr->sin_family = AF_INET;
		*addrlen = sizeof (struct sockaddr_in);

		in_addr->sin_addr.s_addr = utils_htons (socket->get_ipv4_address ().get_host_order ());
		in_addr->sin_port = utils_htons (socket->get_port ());
	}

	return 0;
 error:
	return -1;
}
int 
simu_connect (int fd, struct sockaddr const * addr, 
	      socklen_t addrlen)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
	if (socket->is_ipv6 ()) {
		if (addrlen != sizeof (struct sockaddr_in6)) {
			errno = EINVAL;
			goto error;
		}
		struct sockaddr_in6 *in6_addr = (struct sockaddr_in6 *)addr;
		if (in6_addr->sin6_family != AF_INET6) {
			errno = EAFNOSUPPORT;
			goto error;
		}
		// XXX
		assert (false);
		return -1;
	} else {
		if (addrlen != sizeof (struct sockaddr_in)) {
			errno = EINVAL;
			goto error;
		}
		struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
		if (in_addr->sin_family != AF_INET) {
			errno = EAFNOSUPPORT;
			goto error;
		}
		if (socket->is_connected ()) {
			errno = EISCONN;
			goto error;
		}
		if (socket->is_connecting ()) {
			errno = EALREADY;
			goto error;
		}
		Ipv4Address addr = utils_nstoh (in_addr->sin_addr.s_addr);
		uint16_t port = utils_nstoh (in_addr->sin_port);
		socket->connect (addr, port);
		if (socket->is_connecting ()) {
			errno = EINPROGRESS;
			goto error;
		}
	}
 error:
	return -1;
}
int 
simu_getpeername (int fd, struct sockaddr * addr,
		  socklen_t *addrlen)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
	if (!socket->is_connected ()) {
		errno = ENOTCONN;
		goto error;
	}
	if (socket->is_ipv6 ()) {
		if (*addrlen < sizeof (struct sockaddr_in6)) {
			errno = EINVAL;
			goto error;
		}
		struct sockaddr_in6 *in6_addr = (struct sockaddr_in6 *)addr;
		in6_addr->sin6_family = AF_INET6;
		*addrlen = sizeof (struct sockaddr_in6);
		// XXX
		assert (false);
		return -1;
	} else {
		if (*addrlen < sizeof (struct sockaddr_in)) {
			errno = EINVAL;
			goto error;
		}
		struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
		in_addr->sin_family = AF_INET;
		*addrlen = sizeof (struct sockaddr_in);

		in_addr->sin_addr.s_addr = utils_htons (socket->get_peer_ipv4_address ().get_host_order ());
		in_addr->sin_port = utils_htons (socket->get_peer_port ());
	}
 error:
	return -1;
}
ssize_t 
simu_send (int fd, const void *buf, size_t n, int flags)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
ssize_t 
simu_recv (int fd, void *buf, size_t n, int flags)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
ssize_t 
simu_sendto (int fd, const void *buf, size_t n,
	     int flags, struct sockaddr const * addr,
	     socklen_t addr_len)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
ssize_t 
simu_recvfrom (int fd, void *buf, size_t n,
	       int flags, struct sockaddr * addr,
	       socklen_t *addr_len)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
ssize_t 
simu_sendmsg (int fd, const struct msghdr *message, 
	      int flags)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
ssize_t 
simu_recvmsg (int fd, struct msghdr *message, 
	      int flags)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_getsockopt (int fd, int level, int optname,
		 void *optval,
		 socklen_t *optlen)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_setsockopt (int fd, int level, int optname,
		 const void *optval, socklen_t optlen)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_listen (int fd, int n)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_accept (int fd, struct sockaddr *addr, 
		 socklen_t *addr_len)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_shutdown (int fd, int how)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_sockatmark (int fd)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
int 
simu_isfdtype (int fd, int fdtype)
{
	Socket *socket = socket_lookup (fd);
	if (socket == 0) {
		errno = EBADF;
		goto error;
	}
 error:
	return -1;
}
