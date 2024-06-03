/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 19:36:11 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/03 19:36:11 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <EventLoop.hpp>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <utility>


EventLoop::EventLoop( void ) {
	return ;
}

EventLoop::EventLoop(vector<PortListener>& portVector) {
	_epollFd = epoll_create(1);
	if (_epollFd < 0) {
		throw runtime_error(strerror(errno));
	}
	for (vector<PortListener>::iterator it = portVector.begin();
			it != portVector.end(); ++it) {
		_eventManager.data.fd = (*it).getSocketFd();
		_eventManager.events = EPOLLIN;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _eventManager.data.fd, &_eventManager) < 0) {
			throw runtime_error(strerror(errno));
		}
		_fdMap.insert(pair<const int, PortListener*>(_eventManager.data.fd, &(*it)));
	}
	return ;
}

int	EventLoop::getEpollFd( void ) const {
	return(_epollFd);
}

void	EventLoop::addFdOfInterest(int fd, PortListener* Owner) {
	_fdMap.insert(pair<const int, PortListener *>(fd, Owner));	
	return ;
}

PortListener*	EventLoop::_getOwner(int fd) {
	return (_fdMap.find(fd)->second);
}

void EventLoop::_sigIntCatcher( int signal ) {
	_serverIsRunning = false;
}

void EventLoop::loopForEvent( void ) {
	void (EventLoop::*func)(int);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, _sigIntCatcher);
	_serverIsRunning = true;
	int received_events;

	while (_serverIsRunning) {
		received_events = epoll_wait(_epollFd, &_eventManager, MAX_EVENTS, WAIT_TIMEOUT);

		if (received_events < 0 && errno != EINTR) {
			cout << "Fatal Error with Epoll :"; 
			throw runtime_error(strerror(errno));
		}
		else if (received_events == 0) {
			continue ;
		}
		_getOwner(_eventManager.data.fd)->manageFd(_eventManager.data.fd);
	}
	return ;
}
