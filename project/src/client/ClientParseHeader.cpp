/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientParseHeader.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 13:07:16 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/17 17:14:47 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <string>

void	Client::_parseHeader( void ) {
	string header_line, field_value, field_content;
	while (_header.size() != 0) {
		header_line = _header.substr(0, _header.find("\r\n"));
		_header.erase(0, _header.find("\r\n") + 2);
		substituteSpaces(header_line);
		if (header_line.find(":") == header_line.npos) {
			_noBodyResponseDriver(400, "", true);
			return ;
		}
		field_value = header_line.substr(0, header_line.find_first_of(":"));
		field_content = header_line.substr(header_line.find_first_of(":") + 1, header_line.npos);
		if (field_value.find_first_not_of(" ") != 0) {
			continue;
		}
		transform(field_value.begin(), field_value.end(), field_value.begin(), ::tolower);
		if (fieldValueHasForbiddenChar(field_value) == true) {
			_noBodyResponseDriver(400, "", true);
			return ;
		}
		field_content.erase(0, field_content.find_first_not_of(" "));
		field_content.erase(field_content.find_last_not_of(" ") + 1, field_content.npos);
		_checkHeaderValidity(std::pair<string, string>(field_value, field_content));
		if (_responseIsReady == true) {
			break;
		}
	}
}

void	Client::_checkHeaderValidity( pair<string, string> newHeader) {
	map<string, string>::iterator it = _headerFields.find(newHeader.first);
	if (it == _headerFields.end()) {
		_headerFields.insert(newHeader);
	} else if (newHeader.first == "cookie") {
		it->second += ";" + newHeader.second;
	} else {
			_noBodyResponseDriver(400, "", true);
	}
}

void	Client::_checkForChunkedRequest( void ) {
	const map<string, string>::const_iterator it = _headerFields.find("transfer-encoding"); 
	if (it == _headerFields.end()) {
		_requestIsChunked = false;
		return ;
	}
	if (it->second != "chunked") {
			_noBodyResponseDriver(415, "", true);
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
			_noBodyResponseDriver(400, "", true);
	}
	else if (it->second.size() > 11) {
			_noBodyResponseDriver(400, "", true);
	}
	char *endptr;
	_contentLength = strtol(it->second.c_str(), &endptr, 10);
	if (*endptr != '\0' || _contentLength < 0) {
			_noBodyResponseDriver(400, "", true);
	} else if (_contentLength >= _configServer->get_max_client_body_size() * 1000000) {
			_noBodyResponseDriver(413, "", true);
	}
}

void Client::_parseChunkedRequest() {
	string	chunkSize, chunkBody;
	char		*endptr;
	size_t	numSize;

	while (_chunkedBody.size() != 0) {
		chunkSize = _chunkedBody.substr(0, _chunkedBody.find("\r\n"));
		_chunkedBody.erase(0, chunkSize.size() + 2);
		if (chunkSize.size() > 8) {
			_noBodyResponseDriver(413, "", true);
			break ;
		}
		numSize = strtol(chunkSize.c_str(),&endptr , 16);
		if (numSize == 0) {
			if (_chunkedBody.size() != 2 && _chunkedBody.find("\r\n") != 0) {
				_noBodyResponseDriver(400, "", true);
			} else {
				_bodyIsFullyRed = true;
			}
			break ;
		}
		chunkBody = _chunkedBody.substr(0, numSize);
		if (chunkBody.size() != numSize) {
			_noBodyResponseDriver(400, "", true);
			break ;
		}
		_chunkedBody.erase(0, numSize);
		if (_chunkedBody.find("\r\n") != 0) {
			_noBodyResponseDriver(400, "", true);
			break ;
		}
		_chunkedBody.erase(0, 2);
		_body += chunkBody;
	}
}
