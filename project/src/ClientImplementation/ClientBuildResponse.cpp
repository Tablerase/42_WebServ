/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientBuildResponse.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 12:57:31 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/12 12:57:32 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"

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

void Client::_buildNoBodyResponse(string status, string info, string body, bool isFatal) {
	// string	customPage = _configServer->getCustomStatusPage(status);
	bool		customPageIsPresent = false;
	// if (customPage != "") {
	// 	customPageIsPresent = _loadCustomStatusPage(customPage);
	// }
	if (customPageIsPresent == false) {
		_bodyStream << "<!doctype html><title>" << status << info << "</title><h1>"
			<< info << "</h1><p>" << body << "</p>";
		stringstream size;
		size << _bodyStream.str().size();
		_responseHeader.insert(pair<string, string>("Content-length: ", size.str()));
		_responseHeader.insert(pair<string, string>("Content-type: ", "text/html"));
	}
	_responseHeader.insert(pair<string, string>("Date: ", getDate()));
	_fillResponse(status + info, isFatal);
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
	// _response << "Server: " << _configServer->getName() << "\r\n";
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
	_cgiIsRunning = false;
	if (_cgiInfilePath != "") {
		remove(_cgiInfilePath.c_str());
		_cgiInfilePath = "";
	}
	if (_cgiOutFilePath != "") {
		remove(_cgiOutFilePath.c_str());
		_cgiOutFilePath = "";
	}
}