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
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
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
			_parseHeader();
		}
	}
	if (thisReadAsBeenHandled == false && _responseIsReady == false) {
		if (_bodyIsPresent == false) {
			// error 400
		} else if (_requestIsChunked == true) {
			_parseChunkedRequest(request.substr(bodyStart, request.npos));
		} else {
			_body += request.substr(bodyStart, request.npos);
			if (_body.size() > _contentLength) {
				// error 400
			} else if (_body.size() == _contentLength) {
				_bodyIsFullyRed = true;
			}
		}
	}
	if (_responseIsReady == false && (_bodyIsFullyRed == true || _bodyIsPresent == false)) {
		if (_requestLine.method.compare("GET") == 0) {
			// do get things
		} else if (_requestLine.method.compare("POST") == 0) {
			// do post things
		} else {
			// do delete things
		}
	}
	return ;
}

void Client::_parseChunkedRequest(string requestPart) {
	string	chunk_size, chunk_content;
	size_t	num_size;
	char		*endptr;
	while (requestPart.size() != 0) {
		chunk_size = requestPart.substr(0, requestPart.find("\r\n"));
		if (requestPart.size() > 8) {
			// error 400
			break ;
		}
		requestPart.erase(0, chunk_size.size() + 2);
		num_size = strtol(chunk_size.c_str(), &endptr, 16);
		if (num_size >= _configServer->getMaxBodySize()) {
			// error 413
			break ;
		} else if (*endptr != '\0') {
			// error 400
			break ;
		} else if (num_size == 0) {
			if (requestPart.find("\r\n\r\n") != 0) {
				// error 400
			} else {
				_bodyIsFullyRed = true;
				break ;
			}
		}
		if (requestPart.find("\r\n") != num_size) {
			// error 400
		}
		_body += requestPart.substr(0, num_size);
		requestPart.erase(0, num_size + 2);
	}
	return ;
}

void	Client::_checkContentLength( void ) {
	const map<string, string>::const_iterator it = _headerFields.find("content-length");
	if (it == _headerFields.end()) {
		_contentLength = 0;
		return ;
	} else if (_requestIsChunked == true) {
		// error 400
	}
	else if (it->second.size() > 11) {
		//error 400
	}
	char *endptr;
	_contentLength = strtol(it->second.c_str(), &endptr, 10);
	if (*endptr != '\0' || _contentLength < 0) {
		// error 400
	} else if (_contentLength >= _configServer->getMaxBodySize) {
		// error 413
	}
}

void Client::_manageDeleteRequest( void ) {
	struct stat buffer;	
	if (stat(_requestLine.uri.c_str(), &buffer) != 0) {
		if (errno == EACCES) {
			// error 403
		} else if (errno == ENOENT) {
			// error 404
		} else if (errno == ENOMEM) {
			// error 501
		} else {
			//error 400
		}
	}
	if (!(S_IWUSR & buffer.st_mode)) {
		// error 403
	}
	if (remove(_requestLine.filePath.c_str()) != 0) {
		// 501
	} else {
		// 204
	}
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

void	Client::_checkForChunkedRequest( void ) {
	const map<string, string>::const_iterator it = _headerFields.find("content-encoding"); 
	if (it == _headerFields.end()) {
		_requestIsChunked = false;
		return ;
	}
	if (it->second != "chunked") {
		// error 415
	} else {
		_requestIsChunked = true;
	} return ;
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
	} else if (version < 1.1) {
		// error 426 -> upgrade 1.1
	} else if (*endptr != '\0') {
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
		field_content.erase(0, field_content.find_first_not_of(" "));
		field_content.erase(field_content.find_last_not_of(" ") + 1, field_content.npos);
		if (fieldContentHasForbiddenChar(field_content) == true) {
			//error 404	
		}
		normalizeStr(field_content);
		_headerFields.insert(pair<string, string>(field_value, field_content));
	}
}
