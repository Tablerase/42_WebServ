/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientClassicRequest.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 12:55:10 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/14 19:50:53 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include "color.h"
#include <cstring>
#include <ios>
#include <sys/stat.h>

void Client::_managePostRequest( void ) {
	if (*(_requestLine.absolutePath.end() - 1) == '/') {
		string	index = _locationBlockForTheRequest->index_;
		if (index == "") {
			_noBodyResponseDriver(403, "", false);
			return ;
		} else {
			_requestLine.absolutePath += _locationBlockForTheRequest->index_;
		}
	}
	string extension = _requestLine.absolutePath.substr(_requestLine.absolutePath.find_last_of("."),
			_requestLine.absolutePath.npos);
	map<string,string>::const_iterator it = _locationBlockForTheRequest->cgi_.find(extension);
	if (extension != "" && extension.find("/") == extension.npos
			&& it != _locationBlockForTheRequest->cgi_.end()) {
		_cgiBinPath = it->second;
		_cgiInit();	
	} else {
		_processClassicPostRequest();
	}
}

inline bool	Client::_indexFileExist( const string path) {
	cout << "stating indec" << endl;
	struct stat	buffer;
	const int ret = stat(path.c_str(), &buffer);
	if (ret == 0) {
		cout << "stat succed" << endl;
		return true;
	} else if (errno != ENOENT) {
		cout << "errno" << endl;
		return true;
	} return false;
}

inline bool	Client::_requestIsDir( const string path) {
	cout << "stating indec" << endl;
	struct stat	buffer;
	const int ret = stat(path.c_str(), &buffer);
	if (ret != 0) {
		cout << "stat succed" << endl;
		return false;
	}
	return (S_ISDIR(buffer.st_mode));
}

void	Client::_manageGetRequest( void ) {
	// cout << "The request is :" << _requestLine.filePath << endl;
	if (_requestIsDir(_requestLine.absolutePath) == true) {
		cout << "I got a request ending with a /" << endl;
		string	index = _locationBlockForTheRequest->index_;
		if (index == "") {
			if (_locationBlockForTheRequest->autoindex_ == false) {
				_buildNoBodyResponse("403", " Forbidden", "Access to the ressource is forbidden", false);
			} else {
				_listDirectory();
			}
			return ;
		} else if (_indexFileExist(_requestLine.absolutePath + _locationBlockForTheRequest->index_) == false
				&& _locationBlockForTheRequest->autoindex_ == true){
			cout << "Listing dir" << endl;
			_listDirectory();
			return ;
		} else {
			_requestLine.absolutePath += _locationBlockForTheRequest->index_;
		}
	}
	string extension = _requestLine.absolutePath.substr(_requestLine.absolutePath.find_last_of("."),
			_requestLine.absolutePath.npos);
	map<string,string>::const_iterator it = _locationBlockForTheRequest->cgi_.find(extension);
	if (extension != "" && extension.find("/") == extension.npos
			&& it != _locationBlockForTheRequest->cgi_.end()) {
		cout << RED << "CGI HAS BEEN CALLED" << endl;
		_cgiBinPath = it->second;
		_cgiInit();	
	} else {
		_processClassicGetRequest(extension);
	} 
}

void Client::_statFile(const char* path) {
	struct stat buffer;
	memset(&buffer, 0, sizeof(buffer));
	// cout << path << endl;
	if(stat(path, &buffer) != 0) {
		if (errno == EACCES) {
			_noBodyResponseDriver(403, "", false);
			return ;
		} else if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
			return ;
		} else if (errno == ENOMEM) {
			_noBodyResponseDriver(500, "", false);
			return ;
		} else {
			_noBodyResponseDriver(400, "", true);
			return ;
		}
	}
	// cout << "Get rid of global pb" << endl;
	if (_requestLine.method == "GET" || _requestIsHandledByCgi == true) {
		if (!(S_IRUSR & buffer.st_mode)) {
			_noBodyResponseDriver(403, "", false);
			return ;
		}
		if (S_ISREG(buffer.st_mode) != true) {
			_noBodyResponseDriver(403, "", true);
			return;
		}
		stringstream size;
		size << buffer.st_size;
		_responseHeader.insert(pair<string, string>("Content-length: ", size.str()));
	} else {
		if (!(S_IWUSR & buffer.st_mode)) {
			_noBodyResponseDriver(403, "", false);
		}
	}
}

void	Client::_processClassicGetRequest( string& extension ) {
	_generateContentExtension(extension);	
	if (_checkExtensionMatch(extension) == false) {
		string allowedContent = "Content-Type: " + extension;
			_noBodyResponseDriver(406, allowedContent, true);
		return ;
	} 
	_statFile(_requestLine.absolutePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	ifstream toSend;
	toSend.open(_requestLine.absolutePath.c_str());
	// cout << _requestLine.filePath << boolalpha << toSend.fail() << endl;
	if (toSend.fail()) {
			_noBodyResponseDriver(500, "", false);
	}
	_bodyStream << toSend.rdbuf();
	toSend.close();
	_fillResponse("200 OK", false);
}

void Client::_listDirectory( void ) {
	DIR*	directoryPtr = opendir(_requestLine.absolutePath.c_str());
	if (directoryPtr == NULL) {
		if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
		} else if (errno == EACCES) {
			_noBodyResponseDriver(404, "", false);
		} else {
			_noBodyResponseDriver(500, "", false);
		}
		return ;
	}	
	_bodyStream << "<!DOCTYPE html><html><head><title> Listing of ";
	_bodyStream << _requestLine.filePath << " </title></head><body><p>Content : </p><ul>";
	_requestLine.filePath.erase(_requestLine.filePath.end() - 1);
	for (struct dirent* dirEntry = readdir(directoryPtr);
			dirEntry != NULL; dirEntry = readdir(directoryPtr)) {
		if (dirEntry->d_name[0] == '.') {
			if (strlen(dirEntry->d_name) == 2 && dirEntry->d_name[1] == '.') {
				string pDir = _requestLine.absolutePath;
				if (*(pDir.end() - 1) == '/') {
					pDir.erase(pDir.find_last_of('/'), pDir.npos);
				} pDir.erase(pDir.find_last_of('/'), pDir.npos);
				if (pDir.size() >= _locationBlockForTheRequest->root_.size()) {
					pDir.erase(0, _locationBlockForTheRequest->root_.size());
					_bodyStream << "<li><a href=\"" << pDir << "/\"> ";
					_bodyStream << "..</a></li>";
				}
			}
			continue;
		} else {
			_bodyStream << "<li><a href=\"" << _requestLine.filePath << "/" << dirEntry->d_name;
			if (dirEntry->d_type == DT_DIR) {
				_bodyStream << "/";
			}
			_bodyStream << "\"> " << dirEntry->d_name;
			if (dirEntry->d_type == DT_DIR) {
				_bodyStream << "/";
			}
			_bodyStream << "</a></li>";
		}
	}
	_bodyStream << "</ul></body></html>";
	_responseHeader.insert(pair<string, string>("Content-type: ", "text/html"));
	stringstream size;
	size << _bodyStream.str().size();
	_responseHeader.insert(pair<string, string>("Content-Length: ", size.str()));
	_fillResponse("200 OK", false);
}

void Client::_processClassicPostRequest( void ) {
	_statFile(_requestLine.filePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	ofstream out;	
	out.open(_requestLine.filePath.c_str());
	if (out.fail()) {
			_noBodyResponseDriver(500, "", false);
	} else {
		out << _body;
		out.close();
			_noBodyResponseDriver(201, "", false);
	}
}

void	Client::_manageDeleteRequest( void ) {
	_statFile(_requestLine.filePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	if (remove(_requestLine.filePath.c_str()) != 0) {
			_noBodyResponseDriver(500, "", false);
	} else {
			_noBodyResponseDriver(204, "", false);
	}
}
