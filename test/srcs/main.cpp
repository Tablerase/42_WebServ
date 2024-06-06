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
#include <cctype>
#include <csignal>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <PortListener.hpp>

using namespace std;

int server_is_running;

void	send_index(int new_request);
void sig_handler(int signal);

// TO use me : Compile this and launch it. then open a web browser and type : 127.0.0.1:8080 on a web browser.
// The terminal where the server is launch should print the content of the request and web browser should display content of 
// index.html.

int add_event(int epollfd, PortListener* host) {
	struct epoll_event new_client;
	new_client.events = EPOLLIN;
	new_client.data.fd = host->getSocketFd();
	return (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_client.data.fd, &new_client));
}

PortListener* getGoodListener(PortListener *port1, PortListener *port2, int fd) {
	if (port1->matchFd(fd) == 1) {
		return (port1);
	}
	return (port2);
}

int main() {

	// First we create a socket through a syscall.

	PortListener* port1;
	PortListener* port2;

	try {
		port1 = new PortListener("1312", "jul.html");
	} catch (runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	try {
		port2 = new PortListener("1313", "sch.html");
	} catch (runtime_error& e) {
		delete port1;
		std::cerr << e.what() << std::endl;
		return (1);
	}
	int epollfd = epoll_create(1);
	if (epollfd < 1) {
		perror("Epoll error : ");
		delete port1;
		delete port2;
	}
	struct epoll_event portfds[2];
	portfds[0].data.fd = port1->getPortFd();
	portfds[0].events = EPOLLIN;
	portfds[1].data.fd = port2->getPortFd();
	portfds[1].events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, port1->getPortFd(), &portfds[0]) < 0) {
		delete port1;
		delete port2;
		close (epollfd);
		perror("epoll ctl");
	}
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, port2->getPortFd(), &portfds[1]) < 0) {
		delete port1;
		delete port2;
		close (epollfd);
		perror("epoll ctl");
	}
	server_is_running = 1;
	signal(SIGINT, sig_handler);
	signal(SIGPIPE, SIG_IGN);
	struct epoll_event receiver;
	memset(&receiver, 0, sizeof(receiver));
	cout << "port 1 " << port1->getPortFd() << "port 2 " << port2->getPortFd() << "  " << EPOLLOUT << endl;
	while (server_is_running) {
		// std::cout << "Hey !" << std::endl;
		memset(&receiver, 0, sizeof(receiver));
		int	events = epoll_wait(epollfd, &receiver, 1, 500);
		if (events < 0) {
			perror("Epoll failed");
			break;
		}
		std::cout << "received events : " << events << std::endl;
		if (events == 0) {
			continue;
			std::cout << "Port1 fd : " << port1->getPortFd() << std::endl;
			std::cout << "Port2 fd : " << port2->getPortFd() << std::endl;
			std::cout << "Events Reported fd : " << receiver.data.fd << std::endl;
		}
		if (receiver.data.fd == port1->getPortFd()) {
			std::cout << "Got ya !" << std::endl;
			try {
				if (add_event(epollfd, port1) < 0) {
					perror("epoll ctl");
					break ;
				}
			} catch (runtime_error& e) {
				std::cerr << e.what() << std::endl;
				break ;
			}
		}
		else if (receiver.data.fd == port2->getPortFd()) {
			try {
				if (add_event(epollfd, port2) < 0) {
					perror("epoll ctl");
					break;
				}
			} catch (runtime_error& e) {
				std::cerr << e.what() << std::endl;
				break ;
			}
		}
		else {
		// cout << "Receiver fd : " << receiver.data.fd << "Events : CHeck for epollin" << (receiver.events & EPOLLIN)
			// << "Check for EPOLLOUT " << (receiver.events & EPOLLIN) << "Receiver events raw" << receiver.events << endl;
			PortListener * const of_interest = getGoodListener(port1, port2, receiver.data.fd);
			// cout << of_interest->hasPendingRequest() << endl;
			if ((receiver.events & EPOLLOUT) && of_interest->hasPendingRequest() == 1) {
				cout << "cc" << endl;
				of_interest->writeRequest(receiver.data.fd);
				receiver.events = EPOLLIN;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, receiver.data.fd, &receiver);
			} 
			else if (receiver.events & EPOLLIN) {
				cout << "cc" << endl;
				try {
					of_interest->readRequest(receiver.data.fd);
					receiver.events |= EPOLLOUT;
					epoll_ctl(epollfd, EPOLL_CTL_MOD, receiver.data.fd, &receiver);
				} catch (runtime_error& e) {
					std::cerr << e.what() << std::endl;
					epoll_ctl(epollfd, EPOLL_CTL_DEL, receiver.data.fd, NULL);
				}
			}
			else if ((receiver.events & EPOLLHUP) != 0) {
				cout << "cc" << endl;
				of_interest->closeClient(receiver.data.fd);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, receiver.data.fd, NULL);
				cout << "Finito" << endl;
			}
		}
	}
	delete port1;
	delete port2;
	close (epollfd);
	return (0);
}

void sig_handler(int signal) {
	(void)signal;
	server_is_running = 0;
	return;
}
