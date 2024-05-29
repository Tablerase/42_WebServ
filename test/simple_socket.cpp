/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simple_socket.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 15:53:08 by purmerinos        #+#    #+#             */
/*   Updated: 2024/05/29 15:53:08 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <asm-generic/socket.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

using namespace std;

void	send_index(int new_request);

// TO use me : Compile this and launch it. then open a web browser and type : 127.0.0.1:8080 on a web browser.
// The terminal where the server is launch should print the content of the request and web browser should display content of 
// index.html.

int main() {
	int server_fd;

	// First we create a socket through a syscall.
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("Could not create socket");
		return (1);
	}
	struct sockaddr_in address;

	// sin family : We will use IPv4; Address : We don't care or at least i don't understand
	// it yet. Port is the port we isten too at the moment.
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(8080);

	// This is omly so kernel don't block the port for this socket only.
	int reUse = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse));

	// Assign the socket to the address and port we define with the structure.
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("Could not create bind socket");
		close(server_fd);
		return (1);
	}

	// Reserve the port and start listening to it.
	if (listen(server_fd, 10) < 0) {
		perror("Listen fail");
		close(server_fd);
		return (1);
	}

	int addrlen = sizeof(address);
	while (1) {
		printf("cc\n");

		// BLocking call , this is what we will modify with epoll later.
		int new_request = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		if (new_request < 0) {
			perror("accept fail");
			close(server_fd);
			return (1);
		}

		// We read the request content and print it, but this pretty useless here cause we don't
		// parse it , we will return index.html anyway ...
		char buffer[4096];
		int valread = read(new_request, &buffer, sizeof(buffer) - 1);
		if (valread <= 0) {
			printf("End of transfer\n");
			close(new_request);
			close(server_fd);
			return (0);
		}
		printf("%s\n", buffer);
		send_index(new_request);
	}
	close(server_fd);
	return (0);
}

// I think it is a pretty ugly way to send back some html code lol.
void	send_index(int new_request) {
	// This is a really basic http header for saying ok all good for me , I will send you 
	// back some http code.
	string hello = strdup("HTTP/1.1 200 OK\nContent-Type: text/html text/css\nContent-Length: ");
	char index[335956];
	int fd = open("index.html", O_RDONLY);
	read(fd, index, sizeof(index));
	char size[15];
	sprintf(size, "%ld", strlen(index));
	// We specify the size in byte of the body(e.g the content of index.html). Client will only read
	// up to that number of bytes.
	hello += size;
	// Mandatory separation between header and body ( see RFC)
	hello += "\r\n\r\n";
	hello += index;
	// We write in the socket , this will be done through epoll too cuz for now it is a blocking call.
	write(new_request, hello.c_str(), hello.size());
	close(new_request);
}

