/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/06 13:50:40 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/06 13:50:40 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <utils.hpp>
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
		string to_convert = s.substr(i, i + 2);
		if (to_convert.size() != 3) {
			return (-1);
		}
		int n = strtol(to_convert.c_str(), NULL, 16);
		if (isprint(n) == 0) {
			return (-1);
		}
		char c_str[2];
		if (sprintf(c_str, "%c", n) < 0) {
			return (-1);
		}
		s.replace(i, 3, c_str);
		i = s.find("%", i + 1);
	}
	return (0);
}
