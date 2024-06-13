/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/06 14:01:34 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/06 14:01:35 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP

# define UTILS_HPP

#include <ExternLibrary.hpp>
using namespace std;


void		substituteSpaces(string& s);
int			normalizeStr(string &s);
bool 		fieldValueHasForbiddenChar(const string& s);
bool		fieldContentHasForbiddenChar(const string& s);
string	getDate( void );
void		vectorToCStringTab(const vector<string>& str, vector<char *>& cstr);

#endif
