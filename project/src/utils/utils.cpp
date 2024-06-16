/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/06 13:50:40 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/14 20:10:41 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <string>
#include <utils.hpp>
#include <ExternLibrary.hpp>
#include <Client.hpp>

void	substituteSpaces(string& s) {
	for (string::iterator it = s.begin(), end = s.end(); it != end; ++it) {
		if (*it >= 9 && *it <= 13) {
			*it = ' ';
		}
	}
	return ;
}

int	normalizeStr(string &s) {
	size_t i = s.find("%");
	while (i != s.npos) {
		string to_convert = s.substr(i, 3);
		cout << "New COnvert str : " << to_convert << endl;
		if (to_convert.size() != 3) {
			cout << "Size problem" << endl;
			return (-1);
		}
		to_convert.erase(0, 1);
		cout << "Convert str after trimming % : " << to_convert << endl;
		int n = strtol(to_convert.c_str(), NULL, 16);
		if (isprint(n) == 0) {
			cout << n << "Not printable" << endl;
			return (-1);
		}
		char c_str[2];
		if (sprintf(c_str, "%c", n) < 0) {
			cout << "less then 0" << endl;
			return (-1);
		}
		s.replace(i, 3, c_str);
		i = s.find("%", i + 1);
	}
	return (0);
}

bool fieldValueHasForbiddenChar(const string& s) {
	bool hasForbidden = false;	
	for (size_t i = 0; s[i] != '\0'; ++i) {
		if (isalnum(s[i]) == 0 && s[i] != '-' && s[i] != '_') {
			hasForbidden = true;
			break ;
		}
	}
	return (hasForbidden);
}

static bool loopThroughAllowedChar(char c) {
		if ((c >= ' ' && c <= ';') || c == '=' || (c >= '?' && c <= '[')
				|| c == ']' || c == '_' || (c >= 'a' && c <= 'z') || c == '~') {
			return (true);
		} return (false);
	}

bool fieldContentHasForbiddenChar(const string& s) {
	bool hasForbidden = false;	
	for (size_t i = 0; s[i] != '\0'; ++i) {
		if (loopThroughAllowedChar(s[i]) == false) {
			hasForbidden = true;
			break;
		}
	}
	return (hasForbidden);
}

string	getDate( void ) {
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[128];
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 128, "%a, %d %b %G %T %Z", timeinfo);
	return (buffer);
}

void	vectorToCStringTab(const vector<string>& str, vector<char *>& cstr) {
	cstr.reserve(str.size() + 1);
	for (size_t i = 0; i < str.size(); ++i) {
		cstr.push_back(const_cast<char *>(str[i].c_str()));
	}
	cstr.push_back(NULL);
	return ;
}
