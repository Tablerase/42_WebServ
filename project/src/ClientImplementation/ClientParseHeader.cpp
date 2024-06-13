/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientParseHeader.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 13:07:16 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/12 13:07:19 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"

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
		if (fieldContentHasForbiddenChar(field_content) == true) {
			_noBodyResponseDriver(400, "", true);
			return ;
		}
		normalizeStr(field_content);
		_checkHeaderValidity({field_value, field_content});
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
	const map<string, string>::const_iterator it = _headerFields.find("content-encoding"); 
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
	}// else if (_contentLength >= _configServer->getMaxBodySize) {
	// 		_buildNoBodyResponse("413", " Content Too Large", "Message Body is too large for the server configuration", true);
	// }
}

void Client::_parseChunkedRequest(string requestPart) {
	string	chunk_size, chunk_content;
	size_t	num_size;
	char		*endptr;
	while (requestPart.size() != 0) {
		chunk_size = requestPart.substr(0, requestPart.find("\r\n"));
		if (requestPart.size() > 8) {
			_noBodyResponseDriver(400, "", true);
			break ;
		}
		requestPart.erase(0, chunk_size.size() + 2);
		num_size = strtol(chunk_size.c_str(), &endptr, 16);
		// if (num_size >= _configServer->getMaxBodySize()) {
		// 	_buildNoBodyResponse("413", " Content Too Large", "Message Body is too large for the server configuration", true);
		// 	break ;
	//	}
		if (*endptr != '\0') {
			_noBodyResponseDriver(400, "", true);
			break ;
		} else if (num_size == 0) {
			if (requestPart.find("\r\n\r\n") != 0) {
			_noBodyResponseDriver(400, "", true);
			} else {
				_bodyIsFullyRed = true;
				break ;
			}
		}
		if (requestPart.find("\r\n") != num_size) {
			_noBodyResponseDriver(400, "", true);
		}
		_body += requestPart.substr(0, num_size);
		requestPart.erase(0, num_size + 2);
	}
	return ;
}
