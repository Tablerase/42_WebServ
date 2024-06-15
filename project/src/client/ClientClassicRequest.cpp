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

void	Client::_manageGetRequest( void ) {
	cout << "The request is :" << _requestLine.filePath << endl;
	if (*(_requestLine.filePath.end() - 1) == '/') {
		cout << "I got a request ending with a /" << endl;
		string	index = _locationBlockForTheRequest->index_;
		if (index == "") {
			if (_locationBlockForTheRequest->autoindex_ == false) {
				_buildNoBodyResponse("403", " Forbidden", "Access to the ressource is forbidden", false);
			} else {
				_listDirectory();
			}
			return ;
		} else {
			_requestLine.absolutePath += _locationBlockForTheRequest->index_;
			cout << "Inedexd location" <<  _requestLine.absolutePath << endl;
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
		_processClassicGetRequest(extension);
	} 
}

void Client::_statFile(const char* path) {
	struct stat buffer;
	cout << path << endl;
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
	cout << "Get rid of global pb" << endl;
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
	cout << "A classic Get reauest" << endl;
	_generateContentExtension(extension);	
	if (_checkExtensionMatch(extension) == false) {
		string allowedContent = "Content-Type: " + extension;
		cout << "Bad content ?" << endl;
			_noBodyResponseDriver(406, allowedContent, true);
		return ;
	} 
	cout << "I stat" << endl;
	_statFile(_requestLine.absolutePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	ifstream toSend;
	cout << "stream" << endl;
	toSend.open(_requestLine.filePath.c_str());
	if (toSend.fail()) {
			_noBodyResponseDriver(500, "", false);
	}
	_bodyStream << toSend.rdbuf();
	toSend.close();
	cout << "All good ?" << endl;
	cout << "Stream Content" << _bodyStream.str();
	_fillResponse("200 OK", false);
}

void Client::_listDirectory( void ) {
	DIR*	directoryPtr = opendir(_requestLine.filePath.c_str());
	if (directoryPtr == NULL) {
		if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
		} else if (errno == EACCES) {
			_noBodyResponseDriver(403, "", false);
		} else {
			_noBodyResponseDriver(500, "", false);
		}
	}	
	_bodyStream << "<!DOCTYPE html><html><head><title> Listing of ";
	_requestLine.filePath.erase(_requestLine.filePath.end() - 1);
	_bodyStream << _requestLine.filePath.substr(_requestLine.filePath.find_last_of('/'),
			_requestLine.filePath.npos) << " </title></head><body><p>Content : </p><ul>";
	for (struct dirent* dirEntry = readdir(directoryPtr);
			dirEntry != NULL; dirEntry = readdir(directoryPtr)) {
		if (dirEntry->d_name[0] == '.') {
			continue;
		}
		_bodyStream << "<li><a href=\"" << _requestLine.filePath << dirEntry->d_name;
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