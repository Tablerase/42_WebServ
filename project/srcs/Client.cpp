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

#include <Client.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <utils.hpp>

Client::Client(int fd, PortListener& owner, EventLoop& eventLoop): _connectionEntry(fd), _owner(owner),
	_mainEventLoop(eventLoop), _lastInteractionTime(time(NULL)){
	_eventFunctions[0] = &Client::_newRequest;
	_eventFunctions[1] = &Client::_continueRead;
	_eventFunctions[2] = &Client::_sendAnswer;
	memset(_buffer, 0, BUFFER_SIZE);
	return ;
}

void Client::manageNewEvent( void ) {
	(this->*_eventFunctions[_status])();
}

void	Client::_newRequest( void ) {
	_singleReadBytes = read(_connectionEntry, _buffer, BUFFER_SIZE);	
	if (_singleReadBytes <= 0) {
		throw CloseMeException();
	}
	else if (_singleReadBytes < BUFFER_SIZE) {
		_requestIsFullyRed = true;
	}
	const string request = _buffer;
	memset(_buffer, 0, _singleReadBytes);
	const size_t	endOfHeader = request.find("\r\n\r\n"); 
	if (endOfHeader > MAX_HEADER_SIZE) {
		// 431
		return ;
	}
	_header = request.substr(0, endOfHeader);	
	_body = request.substr(endOfHeader + 4, request.npos);
}

void	Client::_parseRequest( void ) {
	string requestLine = _header.substr(0, _header.find("\r\n"));
	_header.erase(0, requestLine.size() + 2);
	string host = _header.substr(0, _header.find("\r\n"));
	_header.erase(0, host.size() + 2);
	substituteSpaces(requestLine); substituteSpaces(host);
	transform(host.begin(), host.end(), host.begin(), ::tolower);
	if (_header.compare(0, 6, "host: ") != 0) {
		// 400
	}
	_configServer = _owner.getServer(host.substr(host.find_first_of(" "), host.npos));
		
}

void	Client::_parseHeader( void ) {

}
