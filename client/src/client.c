#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int tmp;

void ec(const char* e) {
	if (tmp == -1) {
		perror(e);
		exit(1);
	}
}

void set_nonblocking(int fd, bool nonblocking) {
	tmp = fcntl(fd, F_GETFL);
	ec("fcntl");
	int previous = tmp;

	int new;
	if (nonblocking) {
		new = previous | O_NONBLOCK;
	}
	else {
		new = previous & (~O_NONBLOCK);
	}

	// if (previous != new) {
		tmp = fcntl(fd, F_SETFL, new);
		ec("fcntl");
	// }
}

uint32_t read_num(int sock) {
	int32_t buffer = -1;
	tmp = recv(sock, &buffer, 4, 0);
	return ntohl(buffer);
}

int main(void) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}

	char ip[16];
	short port = 7500;

	printf("IP (127.0.0.1): ");
	fflush(stdout);
	fgets(ip, 16, stdin);
	if (ip[0] == 0) {
		strcpy(ip, "127.0.0.1");
	}
	printf("Port (7500): ");
	fflush(stdout);
	char port_tmp[6];
	fgets(port_tmp, 6, stdin);
	if (port_tmp[0] != 0) {
		sscanf(port_tmp, "%hd", &port);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_aton(ip, &address.sin_addr);

	tmp = connect(sock, (struct sockaddr*)&address, sizeof(address));	
	ec("connect");

	uint32_t prompt_query_length = htonl(8);
	send(sock, &prompt_query_length, 4, 0);
	send(sock, "~prompt\n", 8, 0);

	while (true) {
		switch(0){
		default:
			// Receive
			set_nonblocking(sock, true);
			int32_t fd = read_num(sock);
			if (tmp == -1) {
				set_nonblocking(sock, false);
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					break;
				}
				else {
					ec("read_num");
				}
			}
			set_nonblocking(sock, false);
			if (fd == -1) {
				// Connection ended
				exit(0);
			}
			uint32_t length = read_num(sock);
			// fprintf(stderr, "\x1b[4mclient\x1b[24m: output: %u\n", length);
			char buffer[4097];
			buffer[4096] = 0;
			for (uint32_t i = 0; i < length;) {
				tmp = recv(sock, buffer, length > 4096 ? 4096 : length, 0);
				ec("recv");
				i += tmp;
				// fprintf(stderr, "\x1b[4mclient\x1b[24m: output read: %u\n", i);
				buffer[tmp] = 0;
				if (fd == 1) {
					// fprintf(stderr, "\x1b[4mclient\x1b[24m: stdout\n");
					printf("%s", buffer);
					fflush(stdout);
				}
				else {
					// fprintf(stderr, "\x1b[4mclient\x1b[24m: stderr\n");
					fprintf(stderr, "%s", buffer);
					fflush(stderr);
				}
			}
		}

		switch(0){
		default:;
			// Send
			struct pollfd fds[1];
			fds[0].fd = STDIN_FILENO;
			fds[0].events = POLLIN;
			fds[0].revents = 0;
			poll(fds, 1, 0);
			if (fds[0].revents == 0) {
				break;
			}
			// set_nonblocking(STDIN_FILENO, true);
			tmp = read(STDIN_FILENO, NULL, 0);
			if (tmp == -1) {
				// set_nonblocking(STDIN_FILENO, false);
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					break;
				}
				else {
					ec("read");
				}
			}
			// set_nonblocking(STDIN_FILENO, false);
			char buffer[4097];
			buffer[4096] = 0;
			tmp = read(STDIN_FILENO, buffer, 4096);
			ec("recv");
			buffer[tmp] = 0;
			uint32_t size = tmp;
			// fprintf(stderr, "\x1b[4mclient\x1b[24m: stdin: %u\n", size);
			uint32_t network_size = htonl(size);
			tmp = send(sock, &network_size, 4, 0);
			ec("send");
			for (uint32_t i = 0; i < size;) {
				tmp = send(sock, buffer + i, size - i, 0);
				ec("send");
				// fprintf(stderr, "\x1b[4mclient\x1b[24m: stdin sent: %u\n", i);
				i += tmp;
			}
			send(sock, &prompt_query_length, 4, 0);
			send(sock, "~prompt\n", 8, 0);
		}
	}


	return 0;
}
