#include "wrappers.h"

void print_and_exit(const char *info)
{
	fprintf(stderr, "%s: %s\n", info, strerror(errno));
	exit(EXIT_FAILURE);
}

int Socket(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd < 0)
		print_and_exit("socket error");
	return fd;
}

int Inet_pton(int af, const char *restrict src, void *restrict dst)
{
	int res = inet_pton(af, src, dst);
	if (!res)
	{
		fprintf(stderr, "inet_pton error: network address is incorrect\n");
		exit(EXIT_FAILURE);
	}
	else if (res == -1)
		print_and_exit("inet_pton error");
	return res;
}

int Setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
{
	int res = setsockopt(socket, level, option_name, option_value, option_len);
	if (res < 0)
		print_and_exit("setsockopt error");
	return res;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	ssize_t res = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	if (res < 0)
		print_and_exit("sendto error");
	return res;
}

int Select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout)
{
	int res = select(nfds, readfds, writefds, exceptfds, timeout);
	if (res < 0)
		print_and_exit("select error");
	return res;
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	ssize_t res = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
	if (res < 0)
		print_and_exit("recvfrom error");
	return res;
}

const char *Inet_ntop(int af, const void *restrict src, char *restrict dst, socklen_t size)
{
	const char *res = inet_ntop(af, src, dst, size);
	if (res == NULL)
		print_and_exit("iner_ntop error");
	return res;
}