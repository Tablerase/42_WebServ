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
#include "Server.hpp"
#include "utils.hpp"
#include <algorithm>
#include <ios>
#include <string>
#include <sys/socket.h>

void	Client::_readRequest( void ) {
	bool	thisReadAsBeenHandled = false;
	size_t	bodyStart = 0;
	_status = READING;
	_singleReadBytes = recv(_connectionEntry, _buffer, BUFFER_SIZE, MSG_DONTWAIT);	
	if (_singleReadBytes <= 0) {
		cout << "Failed read " << "Red bytes : " << _singleReadBytes << endl;
		throw CloseMeException();
	}
	const string request(_buffer, _singleReadBytes);
	memset(_buffer, 0, _singleReadBytes);
	if (_headerIsFullyRed == false) {
		// cout << request << endl;
		const size_t endOfHeader = request.find("\r\n\r\n");
		cout << endOfHeader << endl;
		if (endOfHeader == request.npos) {
			_header += request;
			thisReadAsBeenHandled = true;
			return ;
		} else {
			_headerIsFullyRed = true;
			_header += request.substr(0, endOfHeader + 2);
			if (request.begin() + endOfHeader + 4 == request.end()) {
				thisReadAsBeenHandled = true;
			} else {
				bodyStart = endOfHeader + 4;
			}
			_parseRequest();
			cout << "After Parse request" << boolalpha << _responseIsReady << endl;
		}
	}
	if (thisReadAsBeenHandled == false && _responseIsReady == false) {
		cout << "Body is Present : " << boolalpha << _bodyIsPresent << "Request Is Present : " << boolalpha << _requestIsChunked << endl;
		if (_bodyIsPresent == false) {
			_noBodyResponseDriver(400, "", true);
		} else if (_requestIsChunked == true) {
			_parseChunkedRequest(request.substr(bodyStart, request.npos));
		} else {
			_body += request.substr(bodyStart, request.npos);
			if (_body.size() > _contentLength + 2) {
				_noBodyResponseDriver(400, "", true);
			} else if (_body.size() == _contentLength) {
				_bodyIsFullyRed = true;
			}
		}
		cout << "BOdy Size : " << _body.size() << "BOdy Content : " << _body << "Content Length : " << _contentLength << endl;
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
	_copyForDebug = _header;
	string requestLine = _header.substr(0, _header.find("\r\n"));
	_header.erase(0, requestLine.size() + 2);
	string host = _header.substr(0, _header.find("\r\n"));
	_header.erase(0, host.size() + 2);
	substituteSpaces(requestLine); substituteSpaces(host);
	transform(host.begin(), host.end(), host.begin(), ::tolower);
	if (host.compare(0, 6, "host: ") != 0) {
		_noBodyResponseDriver(400, "", true);
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
	string	uri = requestLine.substr(requestLine.find_first_of(" ") + 1, requestLine.npos);
	uri.erase(uri.find_first_of(" "), uri.npos);
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
	if (find(_locationBlockForTheRequest->limit_except_.begin(),
				_locationBlockForTheRequest->limit_except_.end(), _requestLine.method)
			== _locationBlockForTheRequest->limit_except_.end()) {	
		_noBodyResponseDriver(405, "", true);
	}
	if (_locationBlockForTheRequest->redirect_ == true) {
		_responseHeader.insert(pair<string, string>("Location: ", _locationBlockForTheRequest->redirect_path_));
		_noBodyResponseDriver(307, "", true);
	}
	cout << "||||||" << _locationBlockForTheRequest->root_ << "|||||" << endl;
}

void Client::_parseMethod(const string& method) {
	const char *knownMethods[] = {
		"GET", "POST", "DELETE",
		"HEAD", "PUT", "CONNECT",
		"OPTIONS", "TRACE"
	};
	size_t	i;
	for (i = 0; i < 8 && method.compare(knownMethods[i]) != 0; ++i) {}
	if (i <= 2) {
		_requestLine.method = knownMethods[i];
	} else if (i != 8) {
		_noBodyResponseDriver(501, "", true);
	} else {
		_noBodyResponseDriver(400, "", true);
	}
	return ;
}

void Client::_parseUri(const string& uri) {
	if (uri.size() > MAX_URI_SIZE) {
		_noBodyResponseDriver(414, "", true);
	}
	if (uri.find("?") != uri.npos) {
		_requestLine.cgiQuery = uri.substr(uri.find_first_of("?") + 1, uri.npos);
		_requestLine.filePath = uri.substr(0, uri.find_first_of("?"));
	} else {
		_requestLine.filePath = uri;
	}
	if (normalizeStr(_requestLine.filePath) < 0) {
		cout << "NormalizeStr failed" << endl;
		_noBodyResponseDriver(400, "", true);
	}
	_locationBlockForTheRequest = _configServer->get_location(_requestLine.filePath);
	string	rootOfLocation(_locationBlockForTheRequest->root_);
	if (*(rootOfLocation.end() - 1) == '/') {
		rootOfLocation.erase(rootOfLocation.size() - 1, rootOfLocation.npos);
	}
	_requestLine.absolutePath = rootOfLocation + _requestLine.filePath;
}

void Client::_parseProtocol(const string& protocol) {
	if (protocol.size() != 8) {
		_noBodyResponseDriver(400, "", true);
	}
	if (protocol.compare(0, 5, "HTTP/") != 0) {
		_noBodyResponseDriver(400, "", true);
	}
	char *endptr;
	double	version;
	version = strtod(protocol.c_str() + 5, &endptr);
	if (version >= 2.0) {
		_noBodyResponseDriver(505, "", true);
	} else if (version < 1.) {
		_responseHeader.insert(pair<string, string>("Connection: ", "upgrade"));
		_responseHeader.insert(pair<string, string>("Upgrade: ", "HTTP/1.1"));
		_noBodyResponseDriver(426, "", true);
	} else if (*endptr != '\0') {
		_noBodyResponseDriver(400, "", true);
	} else {
		_requestLine.protocol = version;
	} return ;
}
