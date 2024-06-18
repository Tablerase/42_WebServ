/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientClassicRequest.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 12:55:10 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/17 17:25:15 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include <cerrno>
#include <cstring>
#include <dirent.h>
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
	struct stat	buffer;
	const int ret = stat(path.c_str(), &buffer);
	if (ret == 0) {
		return true;
	} else if (errno != ENOENT) {
		return true;
	} return false;
}

inline bool	Client::_requestIsDir( const string path) {
	struct stat	buffer;
	const int ret = stat(path.c_str(), &buffer);
	if (ret != 0) {
		return false;
	}
	return (S_ISDIR(buffer.st_mode));
}

void	Client::_manageGetRequest( void ) {
	cout << _requestLine.absolutePath << endl;
	if (_requestIsDir(_requestLine.absolutePath) == true) {
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
			_listDirectory();
			return ;
		} else {
			_requestLine.absolutePath += _locationBlockForTheRequest->index_;
		}
	}
	string extension = _requestLine.absolutePath.substr(_requestLine.absolutePath.find_last_of("."),
			_requestLine.absolutePath.npos);
	map<string,string>::const_iterator it = _locationBlockForTheRequest->cgi_.find(extension);
	_statFile(_requestLine.absolutePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
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
	memset(&buffer, 0, sizeof(buffer));
	if(stat(path, &buffer) != 0) {
		if (errno == EACCES) {
			_noBodyResponseDriver(403, "", false);
			return ;
		} else if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
			return ;
		} else if (errno == ENOTDIR) {
			_noBodyResponseDriver(409, "", false);
			return ;
		} else if (errno == ENOMEM) {
			_noBodyResponseDriver(500, "", false);
			return ;
		} else {
			_noBodyResponseDriver(400, "", true);
			return ;
		}
	}
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
	ifstream toSend;
	toSend.open(_requestLine.absolutePath.c_str());
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
			_noBodyResponseDriver(403, "", false);
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
					cout << "pDir : " << pDir << endl;
					if (pDir[0] != '/' && pDir.size() != 0) {
						pDir.insert(0, "/");
					}
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
	closedir(directoryPtr);
	_fillResponse("200 OK", false);
}

void Client::_processClassicPostRequest( void ) {
	if (_requestIsDir(_requestLine.absolutePath) == false) {
		string pathForNewLoc = _requestLine.filePath.substr
			(0, _requestLine.filePath.find_last_of("/") + 1);
		string isolatedFile = _requestLine.filePath.substr
			(_requestLine.filePath.find_last_of("/") + 1, _requestLine.filePath.npos);
		_locationBlockForTheRequest = _configServer->get_location(pathForNewLoc);
		if (_locationBlockForTheRequest->upload_path_ != "") {
			_requestLine.absolutePath = _locationBlockForTheRequest->upload_path_;
			if (_requestLine.absolutePath.find_last_of("/")
					!= _requestLine.absolutePath.size() - 1) {
				_requestLine.absolutePath += "/";
			}
			_requestLine.absolutePath += isolatedFile;
		}
	}
	ofstream out;	
	out.open(_requestLine.absolutePath.c_str(), ios_base::app);
	if (out.fail()) {
		if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
		} else if (errno == EACCES) {
			_noBodyResponseDriver(403, "", false);
		} else if (errno == EISDIR) {
			_noBodyResponseDriver(409, "", false);
		} else {
			_noBodyResponseDriver(500, "", false);
		}
	} else {
		out << _body;
		out.close();
			_noBodyResponseDriver(201, "", false);
	}
}

void	Client::_manageDeleteRequest( void ) {
	_statFile(_requestLine.absolutePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	if (remove(_requestLine.absolutePath.c_str()) != 0) {
		_noBodyResponseDriver(500, "", false);
	} else {
		_noBodyResponseDriver(204, "", false);
	}
}
