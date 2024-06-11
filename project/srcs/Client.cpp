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
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <utils.hpp>

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
}

time_t	Client::getLastInteractionTime( void ) const {
	return (_lastInteractionTime);
}

void Client::manageNewEvent( void ) {
	if (_responseIsReady == true) {
		_sendAnswer();
	} else {
		_readRequest();
	}
	time(&_lastInteractionTime);
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
			_parseRequest();
		}
	}
	if (thisReadAsBeenHandled == false && _responseIsReady == false) {
		if (_bodyIsPresent == false) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
		} else if (_requestIsChunked == true) {
			_parseChunkedRequest(request.substr(bodyStart, request.npos));
		} else {
			_body += request.substr(bodyStart, request.npos);
			if (_body.size() > _contentLength + 2) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			} else if (_body.size() == _contentLength + 2) {
				if (_body.find_last_of("\r") != _contentLength && _body.find_last_of("\n") != _contentLength + 1) {
					_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
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
		_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
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
			_buildNoBodyResponse(501, "501 Not Implemeted", "Requested method isn't implemented", false);
	} else {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	}
	return ;
}

void Client::_parseUri(const string& uri) {
	if (uri.size() > MAX_URI_SIZE) {
		_buildNoBodyResponse(413, "413 Uri Too Loong", "Uri exceed max size", true);
	}
	_requestLine.cgiQuery = uri.substr(uri.find_first_of("?", uri.npos));
	if (normalizeStr(_requestLine.filePath) < 0) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	}
	_configServer->getFullPath(_requestLine.filePath);
	if (_configServer.methodIsAllowed(_requestLine.filePath, _requestLine.method) == false) {
			_buildNoBodyResponse(405, "405 Method Not Allowed", "Method is not allowed for the specified route", true);
	}
}

void Client::_parseProtocol(const string& protocol) {
	if (protocol.size() != 8) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	}
	if (protocol.compare(0, 5, "HTTP/") != 0) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	}
	char *endptr;
	double	version;
	version = strtod(protocol.c_str() + 5, &endptr);
	if (version >= 2.0) {
		_buildNoBodyResponse(505, "505 HTTP Protocol not supported", "Server protocol is HTTP/1.1", true);
	} else if (version < 1.) {
		_responseHeader.insert(pair<string, string>("Connection: ", "upgrade"));
		_responseHeader.insert(pair<string, string>("Upgrade: ", "HTTP/1.1"));
		_buildNoBodyResponse(426, "426 Upgrade Required", "This service require use of HTTP/1.1 protocol", true);	
	} else if (*endptr != '\0') {
		_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
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
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			return ;
		}
		field_value = header_line.substr(0, header_line.find_first_of(":"));
		field_content = header_line.substr(header_line.find_first_of(":") + 1, header_line.npos);
		if (field_value.find_first_not_of(" ") != 0) {
			continue;
		}
		transform(field_value.begin(), field_value.end(), field_value.begin(), ::tolower);
		if (fieldValueHasForbiddenChar(field_value) == true) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			return ;
		}
		field_content.erase(0, field_content.find_first_not_of(" "));
		field_content.erase(field_content.find_last_not_of(" ") + 1, field_content.npos);
		if (fieldContentHasForbiddenChar(field_content) == true) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
		}
		normalizeStr(field_content);
		_headerFields.insert(pair<string, string>(field_value, field_content));
	}
}

void	Client::_checkForChunkedRequest( void ) {
	const map<string, string>::const_iterator it = _headerFields.find("content-encoding"); 
	if (it == _headerFields.end()) {
		_requestIsChunked = false;
		return ;
	}
	if (it->second != "chunked") {
		_buildNoBodyResponse(415, "415 Unsupported Media Type", "Only chunked encoding is allowed", true);
	} else {
		_requestIsChunked = true;
	} return ;
}

void	Client::_checkContentLength( void ) {
	const map<string, string>::const_iterator it = _headerFields.find("content-length");
	if (it == _headerFields.end()) {
		_contentLength = 0;
		return ;
	} else if (_requestIsChunked == true) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	}
	else if (it->second.size() > 11) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	}
	char *endptr;
	_contentLength = strtol(it->second.c_str(), &endptr, 10);
	if (*endptr != '\0' || _contentLength < 0) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
	} else if (_contentLength >= _configServer->getMaxBodySize) {
			_buildNoBodyResponse(413, "413 Content Too Large", "Message Body is too large for the server configuration", true);
	}
}

void Client::_managePostRequest( void ) {
	if (*(_requestLine.uri.end() - 1) == '/') {
		string	index = configServer->getIndexFile();
		if (index == "") {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
			return ;
		} else {
			_requestLine.uri += _configServer.getIndexFile();
		}
	}
	string extension = _requestLine.uri.substr(_requestLine.uri.find_last_of("."),
			_requestLine.uri.npos);
	if (extension != "" && extension.find("/") == extension.npos
			&& _configServer->isACgiExtension == true) {
			// do cgi things
	} else {
		_processClassicPostRequest();
	}
}

void Client::_parseChunkedRequest(string requestPart) {
	string	chunk_size, chunk_content;
	size_t	num_size;
	char		*endptr;
	while (requestPart.size() != 0) {
		chunk_size = requestPart.substr(0, requestPart.find("\r\n"));
		if (requestPart.size() > 8) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			break ;
		}
		requestPart.erase(0, chunk_size.size() + 2);
		num_size = strtol(chunk_size.c_str(), &endptr, 16);
		if (num_size >= _configServer->getMaxBodySize()) {
			_buildNoBodyResponse(413, "413 Content Too Large", "Message Body is too large for the server configuration", true);
			break ;
		} else if (*endptr != '\0') {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			break ;
		} else if (num_size == 0) {
			if (requestPart.find("\r\n\r\n") != 0) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			} else {
				_bodyIsFullyRed = true;
				break ;
			}
		}
		if (requestPart.find("\r\n") != num_size) {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
		}
		_body += requestPart.substr(0, num_size);
		requestPart.erase(0, num_size + 2);
	}
	return ;
}

void	Client::_manageGetRequest( void ) {
	if (*(_requestLine.filePath.end() - 1) == '/') {
		string	index = configServer->getIndexFile();
		if (index == "") {
			if (_configServer->isDirectoryListingAllowed == false) {
				_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
			} else {
				_listDirectory();
			}
			return ;
		} else {
			_requestLine.filePath += _configServer.getIndexFile();
		}
	}
	string extension = _requestLine.filePath.substr(_requestLine.uri.find_last_of("."),
			_requestLine.filePath.npos);
	if (extension != "" && extension.find("/") == extension.npos
			&& _configServer->isACgiExtension == true) {
			// do cgi things
	} else {
		_processClassicGetRequest(extension);
	} 
}

void	Client::_processClassicGetRequest( string& extension ) {
	_generateContentExtension(extension);	
	if (_checkExtensionMatch(extension) == false) {
		string allowedContent = "Content-Type: " + extension;
		_buildNoBodyResponse(406, "406 Not Acceptable", allowedContent, false);
		return ;
	} 
	struct stat buffer;
	if(stat(_requestLine.filePath.c_str(), &buffer) != 0) {
		if (errno == EACCES) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
			return ;
		} else if (errno == ENOENT) {
			_buildNoBodyResponse(404, "404 Not Found", "Oops ! It seems there's nothing available here ...", false);
			return ;
		} else if (errno == ENOMEM) {
			_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
			return ;
		} else {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
			return ;
		}
	}
	if (!(S_IRUSR & buffer.st_mode)) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
		return ;
	}
	if (S_ISREG(buffer.st_mode) != true) {
			_buildNoBodyResponse(409, "409 Conflict", "Conflict between the current state of the ressource and the asked one", false);
	}
	stringstream size;
	size << buffer.st_size;
	ifstream toSend;
	toSend.open(_requestLine.filePath);
	if (toSend.fail()) {
		_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
	}
	_responseHeader.insert(pair<string, string>("Content-length: ", size.str()));
	_bodyStream << toSend.rdbuf();
	toSend.close();
	_fillResponse("200 OK", false);
}

void Client::_listDirectory( void ) {
	DIR*	directoryPtr = opendir(_requestLine.filePath.c_str());
	if (directoryPtr == NULL) {
		if (errno == ENOENT) {
			_buildNoBodyResponse(404, "404 Not Found", "Oops ! It seems there's nothing available here ...", false);
		} else if (errno == EACCES) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
		} else {
			_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
		}
	}	
	_requestLine.uri.erase(_requestLine.uri.find_first_of("?"), _requestLine.uri.npos);
	_bodyStream << "<!DOCTYPE html><html><head><title> Listing of ";
	_requestLine.filePath.erase(_requestLine.filePath.end() - 1);
	_bodyStream << _requestLine.filePath.substr(_requestLine.filePath.find_last_of('/'),
			_requestLine.filePath.npos) << " </title></head><body><p>Content : </p><ul>";
	for (struct dirent* dirEntry = readdir(directoryPtr);
			dirEntry != NULL; dirEntry = readdir(directoryPtr)) {
		if (dirEntry->d_name[0] == '.') {
			continue;
		}
		_bodyStream << "<li><a href=\"" << _requestLine.uri << dirEntry->d_name;
		if (dirEntry->d_type == DT_DIR) {
			_bodyStream << "/";
		}
		_bodyStream << "\"> " << dirEntry->d_name;
		if (dirEntry->d_type == DT_DIR) {
			_bodyStream << "/";
		}
		_bodyStream << "</a></li>";
	}
	_bodyStream << "</ul></body></html>";
	_responseHeader.insert(pair<string, string>("Content-type: ", "text/html"));
	stringstream size;
	size << _bodyStream.str().size();
	_responseHeader.insert(pair<string, string>("Content-Length: ", size.str()));
	_fillResponse("200 OK", false);
}

void Client::_generateContentExtension(string& path) {
	path.erase(0, 1);
	if (path == "" || path.find("/") != path.npos) {
		path = "text/plain";
	} else if (path == "jpg" || path == "jpeg" || path == "png"
			|| path == "avif" || path == "webp" ) {
		path.insert(0, "image/");
	} else {
		path.insert(0, "text/");
	}
	_responseHeader.insert(pair<string, string>("Content-type: ", path));
}

bool	Client::_checkExtensionMatch(const string& extension) {
	const map<string, string>::const_iterator it = _headerFields.find("Accept");
	if (it == _headerFields.end()) {
		if (extension != "text/plain" && extension != "text/html") {
			return false;
		} else {
			return true;
		}
	}
	if (it->second.find("*/*") != it->second.npos || it->second.find(extension) != it->second.npos) {
		return true;
	} else {
		return false;
	}
}

void Client::_processClassicPostRequest( void ) {
	struct stat buffer;	
	if (stat(_requestLine.uri.c_str(), &buffer) != 0) {
		if (errno == EACCES) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
		} else if (errno == ENOENT) {
			_buildNoBodyResponse(404, "404 Not Found", "Oops ! It seems there's nothing available here ...", false);
		} else if (errno == ENOMEM) {
			_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
		} else {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
		}
	}
	if (!(S_IWUSR & buffer.st_mode)) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
	}
	ofstream out;	
	out.open(_requestLine.filePath);
	if (out.fail()) {
		_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
	} else {
		out << _body;
		out.close();
		_buildNoBodyResponse(201, "201 Created", "Data Successefully Uploaded", false);
	}
}

void	Client::_manageDeleteRequest( void ) {
	struct stat buffer;	
	if (stat(_requestLine.uri.c_str(), &buffer) != 0) {
		if (errno == EACCES) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
		} else if (errno == ENOENT) {
			_buildNoBodyResponse(404, "404 Not Found", "Oops ! It seems there's nothing available here ...", false);
		} else if (errno == ENOMEM) {
			_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
		} else {
			_buildNoBodyResponse(400, "400 BadRequest", "Syntax error or ambiguous request", true);
		}
	}
	if (!(S_IWUSR & buffer.st_mode)) {
			_buildNoBodyResponse(403, "403 Forbidden", "Access to the ressource is forbidden", false);
	}
	if (remove(_requestLine.filePath.c_str()) != 0) {
		_buildNoBodyResponse(500, "500 Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
	} else {
		_buildNoBodyResponse(204, "204 No content", "No content to display", false);
	}
}

void Client::_buildNoBodyResponse(int status, string info, string body, bool isFatal) {	
	string	customPage = _configServer->getCustomStatusPage(status);
	bool		customPageIsPresent = false;
	if (customPage != "") {
		customPageIsPresent = _loadCustomStatusPage(customPage);
	}
	if (customPageIsPresent == false) {
		_bodyStream << "<!doctype html><title>" << status << info << "</title><h1>"
			<< info << "</h1><p>" << body << "</p>";
		stringstream size;
		size << _bodyStream.str().size();
		_responseHeader.insert(pair<string, string>("Content-length: ", size.str()));
		_responseHeader.insert(pair<string, string>("Content-type: ", "text/html"));
	}
	_responseHeader.insert(pair<string, string>("Date: ", getDate()));
	stringstream statusToStr;
	statusToStr << status;
	_fillResponse(statusToStr.str(), isFatal);
}

bool Client::_loadCustomStatusPage(string path) {
	struct stat buf;
	if (stat(path.c_str(), &buf) != 0) {
		return (0);
	}
	else if (!S_ISREG(buf.st_mode) == false) {
		return (0);
	}
	ifstream customPage;
	customPage.open(path);
	if (customPage.fail()) {
		return (0);
	}
	string extension = path.substr(path.find_last_of("/", path.npos));
	_generateContentExtension(extension);
	if (_checkExtensionMatch(extension) == false) {
		return (0);
	}
	_bodyStream << customPage.rdbuf();
	customPage.close();
	stringstream size;
	size << buf.st_size;
	_responseHeader.insert(pair<string, string>("Content-length: ", size.str()));
	return (1);
}

void	Client::_fillResponse( string status, bool shouldClose ) {
	const map<string, string>::const_iterator it = _responseHeader.find("Connection");
	if (it == _responseHeader.end() || it->second.compare("Keep-Alive") != 0) {
		shouldClose = true;
	}
	if (_responseHeader.find("Connection") == _responseHeader.end()) {
		if (shouldClose == true) {
			_responseHeader.insert(pair<string, string>("Connection: ", "close"));
		} else {
			_responseHeader.insert(pair<string, string>("Connection: ", "Keep-Alive"));
			_responseHeader.insert(pair<string, string>("Keep-Alive: ", "timeout=5, max=1"));
		}
	}
	_response << "HTTP/1.1 " << status << "\r\n";
	_response << "Server: " << _configServer->getName() << "\r\n";
	for (map<string, string>::iterator it = _responseHeader.begin(); it != _responseHeader.end(); ++it) {
		_response << it->first << it->second << "\r\n";
	} _response << "\r\n";
	_response << _bodyStream.rdbuf();
	_response << "\r\n";
	_responseIsReady = true;
	_connectionShouldBeClosed = shouldClose;
	_status = WRITING;
	_mainEventLoop.modifyFdOfInterest(_connectionEntry, EPOLLOUT);
}

void	Client::_sendAnswer( void ) {
	const int writeValue = write(_connectionEntry, _response.str().c_str(), _response.str().size());
	if (writeValue != _response.str().size() || _connectionShouldBeClosed == true) {
		throw CloseMeException();
	}
	_requestLine.cgiQuery.clear();
	_requestLine.filePath.clear();
	_requestLine.method.clear();
	_requestLine.uri.clear();
	_header.clear();
	_body.clear();
	_headerFields.clear();
	_response.clear();
	_bodyStream.clear();
	_responseHeader.clear();
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
}
