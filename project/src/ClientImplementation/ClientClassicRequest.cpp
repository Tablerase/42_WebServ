/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientClassicRequest.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 12:55:10 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/12 12:55:22 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

void Client::_managePostRequest( void ) {
	// if (*(_requestLine.uri.end() - 1) == '/') {
	// 	string	index = configServer->getIndexFile();
	// 	if (index == "") {
	// 		_buildNoBodyResponse("403", " Forbidden", "Access to the ressource is forbidden", false);
	// 		return ;
	// 	} else {
	// 		_requestLine.uri += _configServer.getIndexFile();
	// 	}
	// }
	string extension = _requestLine.uri.substr(_requestLine.uri.find_last_of("."),
			_requestLine.uri.npos);
	// if (extension != "" && extension.find("/") == extension.npos
	// 		&& _configServer->isACgiExtension == true) {
			// do cgi things
	// } else {
		_processClassicPostRequest();
	// }
}

void	Client::_manageGetRequest( void ) {
	if (*(_requestLine.filePath.end() - 1) == '/') {
		//string	index = configServer->getIndexFile();
		// if (index == "") {
		// 	if (_configServer->isDirectoryListingAllowed == false) {
		// 		_buildNoBodyResponse("403", " Forbidden", "Access to the ressource is forbidden", false);
		// 	} else {
				_listDirectory();
	// 		}
	// 		return ;
	// 	} else {
	// 		_requestLine.filePath += _configServer.getIndexFile();
	// 	}
	}
	string extension = _requestLine.filePath.substr(_requestLine.uri.find_last_of("."),
			_requestLine.filePath.npos);
	// if (extension != "" && extension.find("/") == extension.npos
	// 		&& _configServer->isACgiExtension == true) {
	// 		// do cgi things
	// } else {
		_processClassicGetRequest(extension);
	// } 
}

void Client::_statReadOnlyFile(const char* path) {
	struct stat buffer;
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
}

void	Client::_processClassicGetRequest( string& extension ) {
	_generateContentExtension(extension);	
	if (_checkExtensionMatch(extension) == false) {
		string allowedContent = "Content-Type: " + extension;
			_noBodyResponseDriver(403, allowedContent, true);
		return ;
	} 
	_statReadOnlyFile(_requestLine.filePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	ifstream toSend;
	toSend.open(_requestLine.filePath);
	if (toSend.fail()) {
			_noBodyResponseDriver(500, "", false);
	}
	_bodyStream << toSend.rdbuf();
	toSend.close();
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

void Client::_processClassicPostRequest( void ) {
	struct stat buffer;	
	if (stat(_requestLine.uri.c_str(), &buffer) != 0) {
		if (errno == EACCES) {
			_noBodyResponseDriver(403, "", false);
		} else if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
		} else if (errno == ENOMEM) {
			_noBodyResponseDriver(500, "", false);
		} else {
			_noBodyResponseDriver(400, "", true);
		}
	}
	if (!(S_IWUSR & buffer.st_mode)) {
			_noBodyResponseDriver(403, "", false);
	}
	ofstream out;	
	out.open(_requestLine.filePath);
	if (out.fail()) {
			_noBodyResponseDriver(500, "", false);
	} else {
		out << _body;
		out.close();
			_noBodyResponseDriver(201, "", false);
	}
}

void	Client::_manageDeleteRequest( void ) {
	struct stat buffer;	
	if (stat(_requestLine.uri.c_str(), &buffer) != 0) {
		if (errno == EACCES) {
			_noBodyResponseDriver(403, "", false);
		} else if (errno == ENOENT) {
			_noBodyResponseDriver(404, "", false);
		} else if (errno == ENOMEM) {
			_noBodyResponseDriver(500, "", false);
		} else {
			_noBodyResponseDriver(400, "", true);
		}
	}
	if (!(S_IWUSR & buffer.st_mode)) {
			_noBodyResponseDriver(403, "", false);
	}
	if (remove(_requestLine.filePath.c_str()) != 0) {
			_noBodyResponseDriver(500, "", false);
	} else {
			_noBodyResponseDriver(204, "", false);
	}
}
