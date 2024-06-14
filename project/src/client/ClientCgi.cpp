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
#include "utils.hpp"
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

void	Client::_cgiInit( void ) {
	_cgiScriptPath = _requestLine.filePath.substr(0,
			_requestLine.filePath.find_last_of("/") + 1);
	_cgiScriptName = _requestLine.filePath.substr
		(_requestLine.filePath.find_last_of("/"), _requestLine.filePath.npos);

	if (_requestLine.method == "POST") {
		_manageBodyForCgi();
		stringstream infileName;
		infileName << "." << _cgiScriptName << _connectionEntry << "infile";
		_cgiInfilePath = _cgiScriptPath + infileName.str();
	} 
	_manageCgiOutfile();
	stringstream outfileName;
	outfileName << "." << _cgiScriptName << _connectionEntry << "outfile";
	_cgiInfilePath = _cgiScriptPath + outfileName.str();
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
	if (_requestLine.method == "GET") {
		_manageBodyForCgi();
	} _manageCgiOutfile();
	if (chdir(_cgiScriptPath.c_str()) == -1) {
		throw ChildIsExiting();
	}
	_buildEnv();
	vectorToCStringTab(_env, _cEnv);
	//_arg.push_back(server.getCgiPath);
	_arg.push_back(_cgiScriptName);
	vectorToCStringTab(_arg, _cArg);
	execve(_cArg[0], &_cArg[0], &_cEnv[0]);
		_noBodyResponseDriver(500, "", false);
}

void	Client::_buildEnv() {
	_env.push_back("DOCUMENT_ROOT="); // + servergetRoot
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
	_env.push_back("SERVER_NAME="); // + server.getName
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
	const string file = _cgiScriptPath + _cgiInfilePath;
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
	const string file = _cgiScriptPath + _cgiInfilePath;
	const int fd = open(file.c_str(), O_CREAT, O_RDWR);
	if (fd < 0) {
		throw ChildIsExiting();
	}
	if (dup2(fd, STDOUT_FILENO) == -1) {
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
		_buildNoBodyResponse("500", " Internal Server Error", "Sorry, it looks like something went wrong\
on our side ... Maybe try refresh the page ?", false);
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
		_buildNoBodyResponse("500", " Internal Server Error", "Sorry, it looks like something went wrong on our side ... Maybe try refresh the page ?", false);
	}
	_bodyStream << toSend.rdbuf();
	_response << "HTTP/1.1 200 OK\r\n" << _bodyStream.str(); 
	toSend.close();
}
