/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:12:31 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/11 18:00:07 by rcutte           ###   ########.fr       */
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
  std::string path_;

  // Redirections
    // redirection false by default
  bool        redirect_;
  std::string redirect_path_;

  // Path infos

  std::string                         root_;
  std::string                         index_;
    // autoindex - directory listing false by default
  bool                                autoindex_;
    // vector(method) - GET by default (Limited to: GET, POST, DELETE)
  std::vector<std::string>            limit_except_;
  std::string                         upload_path_;
};

class Server
{
private:
  /* Core values */

  std::string name_;
  int         port_;

  /* Additional values */

  int                                 max_client_body_size_;
    // map(error_code, error_page_absolute_path)
  std::map<int, std::string>          error_pages_;
    // map(extension, absolue_path_to_bin)
  std::map<std::string, std::string>  cgi_;

  /* Locations values */
    // map(path, location struct)
  std::map<std::string, location> locations_;

public:
  Server();
  ~Server();

  // Getters

  std::string const &get_name() const;
  int const         &get_port() const;
  std::map<std::string, location> const &get_locations() const;
  location const    &get_location(std::string const &path) const;

  // Setters

  void set_name(std::string const &name);
  void set_port(int const &port);

  // Functions

  void AddLocation(std::string const &path, location const &loc);
  void AddError_page(int const &error_code, std::string const &error_page);
  void AddCgi(std::string const &extension, std::string const &path_to_bin);
};

std::ostream &operator<<(std::ostream &os, const Server &obj);

std::ostream &operator<<(std::ostream &os, const location &obj);

#endif