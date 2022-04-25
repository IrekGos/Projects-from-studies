#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int Socket(int domain, int type, int protocol);
int Inet_pton(int af, const char *restrict src, void *restrict dst);
int Setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
int Select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout);
ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
const char *Inet_ntop(int af, const void *restrict src, char *restrict dst, socklen_t size);

#endif