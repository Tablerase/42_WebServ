/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:12:31 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/11 17:26:39 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
# define SERVER_H_

# include <iostream>
# include <fstream>
# include <iomanip>
# include <stdexcept>
# include <algorithm>
# include <string>
# include <vector>
# include <list>
# include <map>

# include "color.h"

struct location {
  // Path to the location
  std::string path;

  // Redirections
    // redirection false by default
  bool        redirect;
  std::string redirect_path;

  // Path infos

  std::string                         root;
  std::string                         index;
    // autoindex - directory listing false by default
  bool                                autoindex;
    // vector(method) - GET by default (Limited to: GET, POST, DELETE)
  std::vector<std::string>            limit_except;
};

class Server
{
private:
  /* Core values */

  std::string name_;
  int         port_;
  std::string root_;

  /* Additional values */
  
  std::string                         index_;
  int                                 max_client_body_size_;
    // map(error_code, error_page_absolute_path)
  std::map<int, std::string>          error_pages_;
    // map(extension, absolue_path_to_bin)
  std::map<std::string, std::string>  cgi;

  /* Locations values */
    // map(path, location struct)
  std::map<std::string, location> locations_;

public:
  Server();
  ~Server();

  // Getters

  std::string const &get_name() const;
  int const         &get_port() const;

  // Setters

  void set_name(std::string const &name);
  void set_port(int const &port);

  // Functions

  void add_location(std::string const &path, location const &loc);
  void add_error_page(int const &error_code, std::string const &error_page);
  void add_cgi(std::string const &extension, std::string const &path_to_bin);
};

std::ostream &operator<<(std::ostream &os, const Server &obj);

#endif