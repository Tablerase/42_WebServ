/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientCgi.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 20:00:37 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/14 19:49:59 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

void	Client::_cgiInit( void ) {
	_cgiScriptPath = _requestLine.absolutePath.substr(0,
			_requestLine.absolutePath.find_last_of("/") + 1);
	_cgiScriptName = _requestLine.absolutePath.substr
		(_requestLine.absolutePath.find_last_of("/") + 1, _requestLine.absolutePath.npos);

	if (_requestLine.method == "POST") {
		stringstream infileName;
		infileName << "." << _cgiScriptName << _connectionEntry << "infile";
		_cgiInfilePath = _cgiScriptPath + infileName.str();
	} 
	stringstream outfileName;
	outfileName << "." << _cgiScriptName << _connectionEntry << "outfile";
	cout << "Outfile Name : " << outfileName.str() << endl;
	_cgiOutFilePath = _cgiScriptPath + outfileName.str();
	if ((_cgiScriptPid = fork()) == -1) {
		_noBodyResponseDriver(500, "", false);
	} else if (_cgiScriptPid == 0) {
		_childrenRoutine();
	} else {
		_cgiIsRunning = true;
		return ;
	}
}

void	Client::_childrenRoutine() {
	if (_requestLine.method == "POST") {
		_manageBodyForCgi();
	} _manageCgiOutfile();
	if (chdir(_cgiScriptPath.c_str()) == -1) {
		cerr << "chdir failed" << endl;
		throw ChildIsExiting();
	}
	_buildEnv();
	vectorToCStringTab(_env, _cEnv);
	_arg.push_back(_cgiBinPath);
	_arg.push_back(_cgiScriptName);
	vectorToCStringTab(_arg, _cArg);
	execve(_cArg[0], &_cArg[0], &_cEnv[0]);
	cerr << "Execve failed ..." << endl;
	throw ChildIsExiting();
}

void	Client::_buildEnv() {
	_env.push_back("DOCUMENT_ROOT=" + _locationBlockForTheRequest->root_);
	if (_headerFields.find("cookie") != _headerFields.end()) {
		_env.push_back("HTTP_COOKIE" + _headerFields.find("cookie")->second);
	}
	if (_headerFields.find("user-agent") != _headerFields.end()) {
		_env.push_back("HTTP_USER_AGENT" + _headerFields.find("user-agent")->second);
	}
	if (_headerFields.find("accept") != _headerFields.end()) {
		_env.push_back("HTTP_ACCEPT" + _headerFields.find("accept")->second);
	}
	_env.push_back("REQUEST_METHOD" + _requestLine.method);
	_env.push_back("SERVER_SOFTWARE=WebServ/0.1312");
	_env.push_back("SERVER_NAME=" + _configServer->get_name());
	_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_env.push_back("QUERY_STRING=" + _requestLine.cgiQuery);
	if (_requestLine.method == "POST") {
		_env.push_back("CONTENT_LENGTH=" + _infileSize);
		if (_headerFields.find("content-type") != _headerFields.end()) {
			_env.push_back("CONTENT_TYPE" + _headerFields.find("content-type")->second);
		} else {
			_env.push_back("CONTENT_TYPE=text/plain");
		}
	}
}

void	Client::_manageBodyForCgi( void ) {
	const string file = _cgiInfilePath;
	ofstream infile;
	infile.open(file.c_str());
	if (infile.fail()) {
		throw ChildIsExiting();
	}
	infile << _body; 
	stringstream sizeAsStr;
	sizeAsStr << _body.size();
	_infileSize = sizeAsStr.str();
	_body.clear();
	infile.close();
	const int fd = open(file.c_str(), O_CREAT, O_RDWR);
	if (fd < 0) {
		throw ChildIsExiting();
	}
	if (dup2(fd, STDIN_FILENO) == -1) {
		throw ChildIsExiting();
	} close (fd);
}

void	Client::_manageCgiOutfile( void ) {
	const string file = _cgiOutFilePath;
	cout << "Trying to create " << file << endl;
	const int fd = open(file.c_str(), O_CREAT, O_RDWR);
	if (fd < 0) {
		cout << "Outfile Failed" << endl;
		throw ChildIsExiting();
	}
	if (dup2(fd, STDOUT_FILENO) == -1) {
		cout << "Dup2 Failed" << endl;
		throw ChildIsExiting();
	} close (fd);
}

void	Client::_killCgi( void ) {
	kill(_cgiScriptPid, 9);
	if (waitpid(_cgiScriptPid, NULL, 0) < 0) {
		_noBodyResponseDriver(500, "", false);
	}
	_noBodyResponseDriver(504, "", false);
}

void	Client::_checkCgiStatus( void ) {
	int status;
	if (waitpid(_cgiScriptPid, &status, WNOHANG) <= 0) {
		return ;
	} else if (WIFEXITED(status) != true || WEXITSTATUS(status) != 0){
		// _cgiInfilePath = "";
		// _cgiOutFilePath = "";
		_noBodyResponseDriver(500, "", false);
	} else {
		_readOutfile();
	}
	_responseIsReady = true;
	_connectionShouldBeClosed = false;
	_status = WRITING;
	_mainEventLoop.modifyFdOfInterest(_connectionEntry, EPOLLOUT);
	return ;
}

void	Client::_readOutfile( void ) {
	_statFile(_requestLine.filePath.c_str());
	if (_responseIsReady == true) {
		return ;
	}
	ifstream toSend;
	toSend.open(_cgiOutFilePath.c_str());
	if (toSend.fail()) {
		_noBodyResponseDriver(500, "", false);
	}
	_bodyStream << toSend.rdbuf();
	_response << "HTTP/1.1 200 OK\r\n" << _bodyStream.str(); 
	toSend.close();
}
