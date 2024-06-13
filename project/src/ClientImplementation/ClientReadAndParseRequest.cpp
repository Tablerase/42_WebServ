/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientReadAndParseRequest.cpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 12:52:08 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/12 12:53:12 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"

void	Client::_readRequest( void ) {
	bool	thisReadAsBeenHandled = false;
	size_t	bodyStart = 0;
	_status = READING;
	_singleReadBytes = read(_connectionEntry, _buffer, BUFFER_SIZE);	
	if (_singleReadBytes <= 0) {
		throw CloseMeException();
	}
	memset(_buffer, 0, _singleReadBytes);
	const string request(_buffer, _singleReadBytes);
	if (_headerIsFullyRed == false) {
		const size_t endOfHeader = request.find("\r\n\r\n");
		if (endOfHeader == request.npos) {
			_header += request;
			thisReadAsBeenHandled = true;
		} else {
			_header += request.substr(0, endOfHeader);
			if (request.begin() + endOfHeader + 5 == request.end()) {
				thisReadAsBeenHandled = true;
			} else {
				bodyStart = endOfHeader + 5;
			}
			_parseRequest();
		}
	}
	if (thisReadAsBeenHandled == false && _responseIsReady == false) {
		if (_bodyIsPresent == false) {
			_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
		} else if (_requestIsChunked == true) {
			_parseChunkedRequest(request.substr(bodyStart, request.npos));
		} else {
			_body += request.substr(bodyStart, request.npos);
			if (_body.size() > _contentLength + 2) {
			_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
			} else if (_body.size() == _contentLength + 2) {
				if (_body.find_last_of("\r") != _contentLength && _body.find_last_of("\n") != _contentLength + 1) {
					_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
				} else {
					_bodyIsFullyRed = true;
				}
			}
		}
	}
	if (_responseIsReady == false && (_bodyIsFullyRed == true || _bodyIsPresent == false)) {
		if (_requestLine.method.compare("GET") == 0) {
			_manageGetRequest();
		} else if (_requestLine.method.compare("POST") == 0) {
			_managePostRequest();
		} else {
			_manageDeleteRequest();
		}
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
		_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
		return ;
	}
	_configServer = _owner.getServer(host.substr(host.find_first_of(" "), host.find_last_of(":")));	
	_parseRequestLine(requestLine);
	if (_responseIsReady == true) {
		return ;
	} _parseHeader();
	if (_responseIsReady == true) {
		return ;
	}
	_checkForChunkedRequest();
	_checkContentLength();
	if (_requestIsChunked == true || _contentLength > 0) {
		_bodyIsPresent = true;
	}
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
			_buildNoBodyResponse("501", " Not Implemeted", "Requested method isn't implemented", false);
	} else {
			_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
	}
	return ;
}

void Client::_parseUri(const string& uri) {
	if (uri.size() > MAX_URI_SIZE) {
		_buildNoBodyResponse("413", " Uri Too Loong", "Uri exceed max size", true);
	}
	_requestLine.cgiQuery = uri.substr(uri.find_first_of("?", uri.npos));
	if (normalizeStr(_requestLine.filePath) < 0) {
			_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
	}
	//_configServer->getFullPath(_requestLine.filePath);
	// if (_configServer.methodIsAllowed(_requestLine.filePath, _requestLine.method) == false) {
	// 		_buildNoBodyResponse("405", " Method Not Allowed", "Method is not allowed for the specified route", true);
	// }
}

void Client::_parseProtocol(const string& protocol) {
	if (protocol.size() != 8) {
			_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
	}
	if (protocol.compare(0, 5, "HTTP/") != 0) {
			_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
	}
	char *endptr;
	double	version;
	version = strtod(protocol.c_str() + 5, &endptr);
	if (version >= 2.0) {
		_buildNoBodyResponse("505", " HTTP Protocol not supported", "Server protocol is HTTP/1.1", true);
	} else if (version < 1.) {
		_responseHeader.insert(pair<string, string>("Connection: ", "upgrade"));
		_responseHeader.insert(pair<string, string>("Upgrade: ", "HTTP/1.1"));
		_buildNoBodyResponse("426", " Upgrade Required", "This service require use of HTTP/1.1 protocol", true);	
	} else if (*endptr != '\0') {
		_buildNoBodyResponse("400", " BadRequest", "Syntax error or ambiguous request", true);
	} else {
		_requestLine.protocol = version;
	} return ;
}