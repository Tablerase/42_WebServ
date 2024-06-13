/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 19:47:21 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/05 19:47:21 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PortListener.hpp"
#include <Client.hpp>
#include <ctime>

Client::Client(int fd, PortListener& owner, EventLoop& eventLoop): _connectionEntry(fd), _owner(owner),
	_mainEventLoop(eventLoop), _lastInteractionTime(time(NULL)){
	memset(_buffer, 0, BUFFER_SIZE);
	_requestLine.protocol = 0.;
	_status = IDLE;
	_bytesReadFromBody = 0;
	_responseIsReady = false;
	_connectionShouldBeClosed = true;
	_headerIsFullyRed = false;
	_bodyIsFullyRed = false;
	_bodyIsPresent = false;
	_requestIsChunked = false;
	_contentLength = 0;
	return ;
}

Client::~Client( void ) {
	if (_cgiInfilePath != "") {
		remove(_cgiInfilePath.c_str());
	}
	if (_cgiOutFilePath != "") {
		remove(_cgiOutFilePath.c_str());
	}
}

bool	Client::isCLientTimeout( void ) {
	const int timeSinceLastAction = time(NULL) - _lastInteractionTime;
	bool	ret = false;

	if (_lastInteractionTime >= TIMEOUT) {
		if (_cgiIsRunning == false) {
			ret = true;
		} else {
			_killCgi();
			_lastInteractionTime = time(NULL);
		}
	} else {
		if (_cgiIsRunning == true) {
			_checkCgiStatus();
		}
	}
	return (ret);
}

void Client::manageNewEvent( void ) {
	if (_responseIsReady == true) {
		_sendAnswer();
	} else {
		_readRequest();
	}
	time(&_lastInteractionTime);
}
