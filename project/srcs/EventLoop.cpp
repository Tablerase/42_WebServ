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
#include <PortListener.hpp>


EventLoop::EventLoop( void ) {
	return ;
}

EventLoop::EventLoop(vector<PortListener *>& portVector): _PortListenerList(portVector) {
	_epollFd = epoll_create(1);
	if (_epollFd < 0) {
		throw runtime_error(strerror(errno));
	}
	for (vector<PortListener *>::iterator it = _PortListenerList.begin();
			it != _PortListenerList.end(); ++it) {
		_eventManager[0].data.fd = (*it)->getSocketFd();
		(*it)->setMainEventLoop(this);
		_eventManager[0].events = EPOLLIN;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _eventManager[0].data.fd, &_eventManager[0]) < 0) {
			throw runtime_error(strerror(errno));
		}
		_fdMap.insert(pair<const int, PortListener*>(_eventManager[0].data.fd, *it));
	}
	return ;
}

EventLoop::~EventLoop( void ) {
	for (vector<PortListener *>::iterator it = _PortListenerList.begin();
			it != _PortListenerList.end(); ++it) {
		delete *it;
	}
	close(_epollFd);
	return ;
}

int	EventLoop::getEpollFd( void ) const {
	return(_epollFd);
}

void	EventLoop::addFdOfInterest(int fd, PortListener* Owner, int eventsOfInterest) {
	epoll_event	newEvent;
	newEvent.data.fd = fd;
	newEvent.events = eventsOfInterest;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &newEvent) < 0) {
		throw runtime_error(strerror(errno));
	}
	_fdMap.insert(pair<const int, PortListener *>(fd, Owner));
	return ;
}

void	EventLoop::deleteFdOfInterest(int fd) {
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, NULL) < 0) {
		throw runtime_error(strerror(errno));
	}
	_fdMap.erase(fd);
	return ;
}

void	EventLoop::modifyFdOfInterest(int fd, int eventsOfInterest) const {
	epoll_event	to_modify;
	to_modify.data.fd = fd;
	to_modify.events = eventsOfInterest;
	epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &to_modify);
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
		received_events = epoll_wait(_epollFd, _eventManager, MAX_EVENTS, WAIT_TIMEOUT);

		if (received_events < 0 && errno != EINTR) {
			cout << "Fatal Error with Epoll :"; 
			throw runtime_error(strerror(errno));
		}
		else if (received_events == 0) {
			continue ;
		}
		for (int i = 0; i < received_events; ++i) {
			_getOwner(_eventManager[i].data.fd)->manageEvent(_eventManager[i].data.fd);
		}
	}
	return ;
}
