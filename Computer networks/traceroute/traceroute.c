#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include "wrappers.h"

// function from lecture
u_int16_t compute_icmp_checksum(const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t *ptr = buff;
	assert(length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

void print_address_and_time(int packets_received, char (*senders_ip_str)[20], int *times, int iter)
{
	printf("%2d. ", iter);
	if (!packets_received)
		printf("*\n");
	else
	{
		if (strcmp(senders_ip_str[0], "0"))
			printf("%s", senders_ip_str[0]);
		if (strcmp(senders_ip_str[0], senders_ip_str[1]) && strcmp(senders_ip_str[1], "0"))
			printf(" %s", senders_ip_str[1]);
		if (strcmp(senders_ip_str[0], senders_ip_str[2]) && strcmp(senders_ip_str[1], senders_ip_str[2]) && strcmp(senders_ip_str[2], "0"))
			printf(" %s", senders_ip_str[2]);
		if (packets_received == 3)
			printf(" %dms\n", (times[0] + times[1] + times[2]) / 3);
		else
			printf(" ???\n");
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "number of arguments is incorrect\n");
		return EXIT_FAILURE;
	}

	int sockfd = Socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	pid_t pid = getpid();
	int ttl = 1;

	struct icmp header;
	header.icmp_type = ICMP_ECHO;
	header.icmp_code = 0;
	header.icmp_hun.ih_idseq.icd_id = pid;

	struct sockaddr_in recipient;
	bzero(&recipient, sizeof(recipient));
	recipient.sin_family = AF_INET;
	Inet_pton(AF_INET, argv[1], &recipient.sin_addr);

	struct sockaddr_in sender;
	socklen_t sender_len = sizeof(sender);
	u_int8_t buffer[IP_MAXPACKET];

	fd_set descriptors;
	struct timeval tv;

	for (int i = 0; i < 30; i++)
	{
		Setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
		header.icmp_hun.ih_idseq.icd_seq = ttl;
		header.icmp_cksum = 0;
		header.icmp_cksum = compute_icmp_checksum((u_int16_t *)&header, sizeof(header));

		clock_t start, stop;
		int times[3];
		int packets_received = 0;
		char senders_ip_str[3][20] = {{'0'}, {'0'}, {'0'}};

		FD_ZERO(&descriptors);
		FD_SET(sockfd, &descriptors);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		bool finished = false;

		Sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr *)&recipient, sizeof(recipient));
		Sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr *)&recipient, sizeof(recipient));
		Sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr *)&recipient, sizeof(recipient));

		for (int j = 0; j < 3; j++)
		{
			start = clock();
			if (Select(sockfd + 1, &descriptors, NULL, NULL, &tv))
			{
				Recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len);
				stop = clock();
				times[j] = (double)(stop - start) / CLOCKS_PER_SEC * 1000000;

				struct ip *ip_header = (struct ip *)buffer;
				ssize_t ip_header_len = 4 * ip_header->ip_hl;
				uint8_t *icmp_packet = buffer + ip_header_len;
				struct icmp *icmp_header = (struct icmp *)icmp_packet;
				if (icmp_header->icmp_type == ICMP_ECHOREPLY)
				{
					if (icmp_header->icmp_id == pid && icmp_header->icmp_seq == ttl)
					{
						finished = true;
						packets_received++;
						Inet_ntop(AF_INET, &(sender.sin_addr), senders_ip_str[j], sizeof(senders_ip_str));
					}
					else
						j--;
				}
				else if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED)
				{
					struct ip *original_ip_header = (struct ip *)buffer + ip_header_len + sizeof(struct icmp); // original IP header is in ICMP data
					ssize_t original_ip_header_len = 4 * original_ip_header->ip_hl;
					uint8_t *original_icmp_packet = buffer + ip_header_len + sizeof(struct icmp) + original_ip_header_len;
					struct icmp *original_icmp_header = (struct icmp *)original_icmp_packet;
					if (original_icmp_header->icmp_id == pid && original_icmp_header->icmp_seq == ttl)
					{
						packets_received++;
						Inet_ntop(AF_INET, &(sender.sin_addr), senders_ip_str[j], sizeof(senders_ip_str));
					}
					else
						j--;
				}
			}
		}
		print_address_and_time(packets_received, senders_ip_str, times, ttl);

		if (finished)
			return EXIT_SUCCESS;

		ttl++;
	}
	return 0;
}