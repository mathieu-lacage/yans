/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef SOCKET_H
#define SOCKET_H

int socket(int domain, int type, int protocol);
int bind(int sockfd, struct sockaddr *my_addr, 
	 socklen_t addrlen);
int getsockname (int fd, struct sockaddr *addr, 
		       socklen_t *len);
int connect (int fd, struct sockaddr const * addr, 
		   socklen_t len);
int getpeername (int fd, struct sockaddr * addr,
		       socklen_t *len);
ssize_t send (int fd, const void *buf, size_t n, int flags);
ssize_t recv (int fd, void *buf, size_t n, int flags);
ssize_t sendto (int fd, const void *buf, size_t n,
		int flags, struct sockaddr const * addr,
		socklen_t addr_len);
ssize_t recvfrom (int fd, void *buf, size_t n,
			int flags, struct sockaddr * addr,
			socklen_t *addr_len);
ssize_t sendmsg (int fd, const struct msghdr *message, 
		       int flags);
ssize_t recvmsg (int fd, struct msghdr *message, 
		       int flags);
int getsockopt (int fd, int level, int optname,
		      void *optval,
		      socklen_t *optlen);
int setsockopt (int fd, int level, int optname,
		      const void *optval, socklen_t optlen);
int listen (int fd, int n);
int accept (int fd, struct sockaddr *addr, 
		  socklen_t *addr_len);
int shutdown (int fd, int how);
int sockatmark (int fd);
int isfdtype (int fd, int fdtype);


#endif /* SOCKET_H */
