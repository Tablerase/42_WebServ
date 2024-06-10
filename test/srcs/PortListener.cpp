/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/30 17:48:24 by purmerinos        #+#    #+#             */
/*   Updated: 2024/05/30 17:48:24 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <PortListener.hpp>
#include <asm-generic/socket.h>
#include <csignal>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

PortListener::PortListener(const char* port, const string& index) : _index(index) {
	_portFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_portFd < 0) {
		throw runtime_error(strerror(errno));
		return;
	}

	struct addrinfo		hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int	gai_error = getaddrinfo(NULL, port, &hints, &_res);

	if (gai_error != 0) {
		throw runtime_error(gai_strerror(gai_error));
		return;
	}
	// This is only so kernel don't block the port for this socket only.
	int reUse = 1;
	setsockopt(_portFd, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse));

	// Assign the socket to the address and port we define with the structure.

	for (_address = _res; _address != NULL; _address = _address->ai_next) {
		if (bind(_portFd, _address->ai_addr, _address->ai_addrlen) == 0) {
			break;
		}
	}
	if (_address == NULL) {
		freeaddrinfo(_res);
		_res = NULL;
		close(_portFd);
		throw runtime_error(strerror(errno));
	}

	// Reserve the port and start listening to it.
	if (listen(_portFd, 4096) < 0) {
		throw runtime_error(strerror(errno));
		close(_portFd);
		return;
	}
}

PortListener::~PortListener( void ) {
	for (vector<int>::iterator it = _clientFds.begin(); it != _clientFds.end(); ++it) {
		close(*it);
	}
	close(_portFd);
	if (_res != NULL) {
		freeaddrinfo(_res);
	}
}

int	PortListener::getPortFd(void) const {
	return(this->_portFd);
}

int PortListener::matchFd(int fd) const {
	for (vector<int>::const_iterator it = _clientFds.begin();
			it != _clientFds.end(); ++it) {
		if (*it == fd) {
			return (1);
		}
	}
	return (0);
}

int PortListener::hasPendingRequest( void ) const {
	if (this->_request.size() != 0) {
		return (1);
	}
	return (0);
}

int PortListener::getSocketFd( void ) {
	cout << "I have been called lol" << endl;
	const int socketFd = accept(this->_portFd, this->_address->ai_addr,
			(socklen_t *)&this->_address->ai_addrlen);
	if (socketFd < 0) {
		throw runtime_error(strerror(errno));
	}
	_clientFds.push_back(socketFd);
	return (socketFd);
}

void	PortListener::readRequest(int fd) {
	char	buffer[4096];

	cout << "client" << fd << endl;
	int bytesRead = recv(fd, buffer, 4095, 0);
	if (bytesRead <= 0) {
		cout << "Nothing to read" << endl;
		closeClient(fd);
		throw runtime_error(strerror(errno));
		return ;
	}
	this->_request += buffer;
	std::cout << buffer << std::endl;
	memset(buffer, 0, 4096);
}

void	PortListener::closeClient( int fd ) {
	close(fd);
	vector<int>::iterator it = find(_clientFds.begin(), _clientFds.end(), fd); 
	_clientFds.erase(it);
	return ;
}

void PortListener::writeRequest(int fd) {
	ifstream index_file;
	index_file.open(this->_index.c_str());
	if (index_file.fail()) {
		throw runtime_error("Failed to open file");
	}
	index_file.seekg(0, index_file.end);
	int file_size = index_file.tellg();
	index_file.seekg(0, index_file.beg);
	_response = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=UTF-8\nContent-Length: ";
	char size[12];
	sprintf(size, "%d", file_size);
	_response += size;
	_response += "\r\n\r\n";
	ostringstream sstream;
	 << index_file.rdbuf();
	_response += sstream.str();
	cout << _response << endl;
	write(fd, _response.c_str(), _response.size());
	_response.clear();
	_request.clear();
}
