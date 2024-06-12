/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 18:01:00 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/11 18:04:43 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ======================== Constructor / Destructor ======================== */

/* ================================ Getters ================================= */

/* ================================ Setters ================================= */

/* =============================== Functions ================================ */

/* =============================== Operators ================================ */

/* ============================= Output stream ============================== */

std::ostream &operator<<(std::ostream &out, const location &obj){
  out
    << "Location: " << obj.path_ << std::endl
    << "Redirect: " << obj.redirect_ << std::endl
    << "Redirect path: " << obj.redirect_path_ << std::endl
    << "Root: " << obj.root_ << std::endl
    << "Index: " << obj.index_ << std::endl
    << "Autoindex: " << obj.autoindex_ << std::endl
    << "Limit except: ";
  for (std::vector<std::string>::const_iterator i = obj.limit_except_.begin(); i != obj.limit_except_.end(); ++i)
    out << *i << ' ';
  out << std::endl;
  out << "Upload path: " << obj.upload_path_ << std::endl;
  for (map<string,string>::const_iterator it = obj.cgi_.begin(); it != obj.cgi_.end(); ++it)
    out << "CGI: " << it->first << " -> " << it->second << endl;
  return out;
}
