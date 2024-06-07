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
#include <cstddef>
#include <cstdlib>
#include <string>
#include <utils.hpp>

Client::Client(int fd, PortListener& owner, EventLoop& eventLoop): _connectionEntry(fd), _owner(owner),
	_mainEventLoop(eventLoop), _lastInteractionTime(time(NULL)){
	memset(_buffer, 0, BUFFER_SIZE);
	return ;
}

void Client::manageNewEvent( void ) {
	if (_responseIsReady == true) {
		_sendAnswer();
	} else {
		_readRequest();
	}
}

void	Client::_readRequest( void ) {
	_status = READING;
	_singleReadBytes = read(_connectionEntry, _buffer, BUFFER_SIZE);	
	if (_singleReadBytes <= 0) {
		throw CloseMeException();
	}
	memset(_buffer, 0, _singleReadBytes);
	const string request(_buffer, _singleReadBytes);
	if (_headerIsFullyRed == false) {
		const size_t	endOfHeader = request.find("\r\n\r\n"); 
		_header += request.substr(0, endOfHeader);	
		_body += request.substr(endOfHeader + 4, request.npos);
		if (_header.size() > MAX_HEADER_SIZE) {
			// 431
			return ;
		}
		if (endOfHeader != request.npos) {
			_headerIsFullyRed = true;
			_parseRequest();
		}
	} else {
		_body += request;
		// Do body things.
	}
	return ;
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
		return ;
	}
	_configServer = _owner.getServer(host.substr(host.find_first_of(" "), host.find_last_of(":")));	
	_parseRequestLine(requestLine);
	if (_responseIsReady == true) {
		return ;
	} _parseHeader();
	return ;
}

void	Client::_parseRequestLine( const string& requestLine) {
	const string	method = requestLine.substr(0, requestLine.find_first_of(" "));
	const string	uri = requestLine.substr(requestLine.find_first_of(" ") + 1, requestLine.find_last_of(" "));
	const string	protocol = requestLine.substr(requestLine.find_last_of(" ") + 1, requestLine.npos);
	
	_parseProtocol(protocol);
	if (_responseIsReady == true) {
		return ;
	} _parseMethod(method);
	if (_responseIsReady == true) {
		return ;
	} _parseUri(uri);
	if (_responseIsReady == true) {
		return ;
	}
}

void Client::_parseMethod(const string& method) {
	const char *knownMethods[] = {
		"GET", "POST", "DELETE",
		"HEAD", "PUT", "CONNECT",
		"OPTIONS", "TRACE"
	};
	size_t	i;
	for (i = 0; i < 8 && method.compare(knownMethods[i]) != 0; ++i) {}
	if (i < 2) {
		_requestLine.method = knownMethods[i];
	} else if (i != 8) {
		// error 501
	} else {
		// error 400
	}
	return ;
}

void Client::_parseUri(const string& uri) {
	if (uri.size() > MAX_URI_SIZE) {
		// error 414
	}
	_requestLine.filePath = uri.substr(0, uri.find_first_of("?"));
	_requestLine.cgiQuery = uri.substr(uri.find_first_of("?", uri.npos));
	if (normalizeStr(_requestLine.filePath) < 0) {
		// error 400
	}
	// server.getFullPath(_requestLine.filePath);
	// if (server.methodIsAllowed(_requestLine.fullPath, _requestLine.method) == false) {
		// error 405
	//}
}

void Client::_parseProtocol(const string& protocol) {
	if (protocol.size() != 8) {
		// error 400
	}
	if (protocol.compare(0, 5, "HTTP/") != 0) {
		// error 400
	}
	char *endptr;
	double	version;
	version = strtod(protocol.c_str() + 5, &endptr);
	if (version > 1.1) {
		// error 505
	} else if (version < 0.9 || *endptr != '\0') {
		// error 400
	} else {
		_requestLine.protocol = version;
	} return ;
}

void	Client::_parseHeader( void ) {
	string header_line, field_value, field_content;
	while (_header.size() != 0) {
		header_line = _header.substr(0, _header.find("\r\n"));
		_header.erase(0, _header.find("\r\n") + 2);
		substituteSpaces(header_line);
		if (header_line.find(":") == header_line.npos) {
			// error 400
			return ;
		}
		field_value = header_line.substr(0, header_line.find_first_of(":"));
		field_content = header_line.substr(header_line.find_first_of(":") + 1, header_line.npos);
		if (field_value.find_first_not_of(" ") != 0) {
			continue;
		}
		transform(field_value.begin(), field_value.end(), field_value.begin(), ::tolower);
		if (fieldValueHasForbiddenChar(field_value) == true) {
			//error 404
			return ;
		}
		normalizeStr(field_content);
		field_content.erase(0, field_content.find_first_not_of(" "));
		field_content.erase(field_content.find_last_not_of(" ") + 1, field_content.npos);
		if (fieldContentHasForbiddenChar(field_content) == true) {
			//error 404	
		}
		_headerFields.insert(pair<string, string>(field_value, field_content));
	}
}
