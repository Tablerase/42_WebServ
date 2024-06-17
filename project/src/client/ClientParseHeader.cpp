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
	} else if (_contentLength >= _configServer->get_max_client_body_size() * 1000000) {
			cout << "Max Body Size" << _configServer->get_max_client_body_size() << endl;
			_noBodyResponseDriver(413, "", true);
	}
}

void Client::_parseChunkedRequest(string requestPart) {
	string	chunk_size, chunk_content;
	size_t	num_size;
	char		*endptr;
	cout << "ENtering Chunk Parsing With Request : " << requestPart << endl;
	cout << "beg of part request : " << requestPart.size() << endl;
	if (requestPart.find("\r\n") == 0) {
		requestPart.erase(0, 2);
	}
	while (requestPart.size() != 0) {
		chunk_size = requestPart.substr(0, requestPart.find("\r\n"));
		cout << "Chunk Size: " << chunk_size << endl;
		if (chunk_size.size() > 8) {
			cout << "Size" << endl;
			_noBodyResponseDriver(400, "", true);
			break ;
		}
		requestPart.erase(0, chunk_size.size() + 2);
		cout << "AFter erasing chunk part, reauest line is : " << requestPart << endl;
		num_size = strtol(chunk_size.c_str(), &endptr, 16);
		cout << "Strtol Receive : " << chunk_size << "And Return : " << num_size << endl;
		if (num_size >= _configServer->get_max_client_body_size() * 1000000) {
			_noBodyResponseDriver(413, "", true);
			break ;
	}
		if (*endptr != '\0') {
			cout << "endptr" << *endptr << endl;
			_noBodyResponseDriver(400, "", true);
			break ;
		} else if (num_size == 0) {
			cout << "Remaining Request : " << requestPart << "Result of find " << requestPart.find("\r\n") << "RequestPart Size " << requestPart.size() << endl;
			if (requestPart.find("\r\n") != 0) {
			_noBodyResponseDriver(400, "", true);
			break ;
			} else {
				_bodyIsFullyRed = true;
				break ;
			}
		}
		chunk_content = requestPart.substr(0, num_size);
		if (chunk_content.size() != num_size) {
			cout << "I failed right there because chunk_content size is : " << chunk_content.size() << " and num size is : " << num_size << endl;
			cout << "Chunk Content :" << chunk_content << endl;
			_noBodyResponseDriver(400, "", true);
			break;
		}
		_body += chunk_content;
		requestPart.erase(0, num_size + 2);
		if (requestPart.find("\r\n") != 0) {
			cout << "Found cr = " << requestPart.find("\r\n") << endl;
			_noBodyResponseDriver(400, "", true);
			break;
		}
		cout << "End of a loop cycle, remaining reauest part : " << requestPart.size() << endl;
		cout << "And Body is currently : " << _body;
	}
	cout << "End of part request : " << requestPart.size() << endl;
	return ;
}
